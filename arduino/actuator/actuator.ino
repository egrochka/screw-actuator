// CONTROL NEMA23 STEPPER MOTOR USING A LOADCELL INPUT

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <HX711.h>
#include <LiquidCrystal.h>
#include <string>

// Define pins for input/output
#define PIN_LED1 0
#define PIN_SWITCH 1
#define PIN_CW1 2
#define PIN_CW2 3
#define PIN_CCW1 4
#define PIN_CCW2 5
#define PIN_CLK 6
#define PIN_DT 7
#define PIN_DISP_DT4 11
#define PIN_DISP_DT5 10
#define PIN_DISP_DT6 9
#define PIN_DISP_DT7 8
#define PIN_DISP_RS 12
#define PIN_DISP_EN 13
#define PIN_POT A0

// Define other variables
int load_min = 0;
int load_max = 8000;
float pos_home;
int weight_cal = 109;  // Calibration load weight in grams

// Various speeds
int speed_min = 0;
int speed_max = 5000;
int speed_main = 0;
int speed_return = -3000;

// Create objects from libraries
AccelStepper stepper;
HX711 cell;
LiquidCrystal lcd(PIN_DISP_RS, PIN_DISP_EN, PIN_DISP_DT4, PIN_DISP_DT5, PIN_DISP_DT6, PIN_DISP_DT7);

// CLASSES ------------------------------------------------------------------------

class Page {

private:

  static int const _chars = 20;
  static int const _rows = 4;
  String _name;
  float _nums[_rows];
  String _msgs[_rows];

  void deletefrom(int row, int pos) {
    int chars_max = _chars;
    if (row >= 2) {
      pos += 16;
      chars_max += 16;
    }
    for (int i = pos; i < chars_max; i++) {
      lcd.setCursor(i, row);
      lcd.print(" ");
    }
  }

  void drawLine(int pos = 0, int row = 0, String line = "") {
    if (row >= 2) {
      pos += 16;
    }
    //deletefrom(row, pos); // Maybe replace with deletefrom later
    lcd.setCursor(pos, row);
    lcd.print("                    ");
    lcd.setCursor(pos, row);
    lcd.print(line);
  }

public:

  Page() {}  // DO NOT USE

  Page(String name,
       String msg0 = "",
       String msg1 = "",
       String msg2 = "",
       String msg3 = "",
       float num0 = NAN,
       float num1 = NAN,
       float num2 = NAN,
       float num3 = NAN) {

    _name = name;

    _nums[0] = num0;
    _nums[1] = num1;
    _nums[2] = num2;
    _nums[3] = num3;

    _msgs[0] = msg0;
    _msgs[1] = msg1;
    _msgs[2] = msg2;
    _msgs[3] = msg3;
  }

  void begin() {
    lcd.begin(_rows, _chars);
    delay(1000);
    lcd.clear();
  }

  void pageDraw() {
    for (int i = 0; i < _rows; i++) {
      if (_nums[i] != _nums[i]) {
        drawLine(0, i, _msgs[i]);
      } else {
        drawLine(0, i, _msgs[i] + String(_nums[i]));
      }
    }
  }

  // void pageUpdate() {
  //   for (int i = 0; i < _rows; i++) {
  //     if (!(_nums[i] != _nums[i])) {
  //       int len = _msgs[i].length();
  //       drawLine(len, i, String(_nums[i]));
  //     }
  //   }
  // }
  // FINISH LATER

  String getName() {
    return _name;
  }
  String getMsg(int num) {
    return _msgs[num];
  }
  void editName(String edit) {
    _name = edit;
  }
  void editMsg(int num, String edit) {
    _msgs[num] = edit;
  }
  void editNum(int num, float edit) {
    _nums[num] = edit;
  }
};

Page refPage("Reference mode",
             "---REFERENCE MODE---",
             "Reference: ",
             "",
             "");

Page runPage("Run mode",
             "------RUN MODE------",
             "Reference: ",
             "Reading: ",
             "Motor speed: ");

Page conPage("Confirmation page",
             "Confirm start?",
             "",
             "",
             "");

Page calPage("Calibration page",
             "--CALIBRATION MODE--",
             "",
             "",
             "");

Page menPage("Menu page",
             refPage.getName(),
             runPage.getName(),
             calPage.getName());

int const entrys_num = 3;
Page entrys[entrys_num] = { refPage, runPage, calPage };

class LED {

private:
  int _pin;

public:
  LED() {}  //DO NOT USE

  LED(int pin) {
    _pin = pin;
  }
  void on() {
    digitalWrite(_pin, HIGH);
  }
  void off() {
    digitalWrite(_pin, LOW);
  }
};


class Controller {
  // Controller parameters

private:
  float Kp = 10;
  float Ki = 0;
  float Kd = 1;//1.0;
  float integral = 0;

public:

  int _update_time = 50;

  Controller() {}

  float PID(int load_ref, int load_in, int load_old, long dt) {

    int err = (load_ref - load_in);
    float P = Kp * err;

    integral += err * dt;
    float I = Ki * integral;  // constrain(integral, load_min, load_max);

    int dErr = (load_in - load_old) / dt;
    float D = Kd * dErr;

    float C = P + I + D;
    C = constrain(C, -load_ref, load_ref);
    int speed = map(C, load_min, load_ref, speed_min, speed_max);

    //Serial.println("P: " + String(P) + " I: " + String(I) + " D: " + String(D) + " C: " + String(C) + " Speed: " + String(speed));
    Serial.println(String(load_in) + " " + String(millis()) + " " + String(load_ref) + " " + String(speed));
    return speed;
  }

  float reset() {
    integral = 0;
  }
};

// Create objects from classes
Controller controller;
LED LED1(PIN_LED1);

// FUNCTIONS ------------------------------------------------------------------------

bool pressed = false;
void interruptStop() {
  pressed = true;
  delay(300);
}

void calibrate() {
  // Calibrate loadcell before use

  calPage.editMsg(1, "Taring...");
  calPage.pageDraw();
  cell.tare(20);
  float offset = cell.get_offset();

  calPage.editMsg(1, "Hang cal. weight");
  calPage.pageDraw();

  while (!pressed) delay(10);
  pressed = false;
  cell.calibrate_scale(weight_cal, 5);

  calPage.editMsg(1, "Done!");
  calPage.editMsg(2, "Cal. scale: ");
  calPage.editNum(2, cell.get_scale());
  calPage.pageDraw();

  delay(1000);
  return;
}

// Variables stored between loops
float load_in = cell.get_units();
float load_ref;
bool switch_in = false;
long time_current = millis();

void runMode() {

  int timing = 0;

  LED1.on();

  while (true) {

    long time_old = time_current;
    time_current = millis();
    long dt = time_current - time_old;
    timing += dt;

    if (timing >= controller._update_time) {

      cell.wait_ready();

      float load_old = load_in;
      load_in = cell.get_units();
      speed_main = controller.PID(load_ref, load_in, load_old, dt);
      stepper.setSpeed(speed_main);

      //runPage.editNum(1, load_ref);
      //runPage.editNum(2, load_in);
      //runPage.editNum(3, speed_main);
      //runPage.pageDraw();

      timing = 0;
    }

    stepper.runSpeed();

    if (pressed) {
      pressed = false;

      stepper.setSpeed(speed_return);
      while (stepper.currentPosition() < pos_home) {
        stepper.runSpeed();
      }

      LED1.off();
      return;
    }
  }
}

void refMode() {

  int timing = 0;
  controller.reset();

  while (true) {
    long time_old = time_current;
    time_current = millis();
    long dt = time_current - time_old;
    timing += dt;

    load_ref = (analogRead(PIN_POT) / 1023.0) * load_max;

    if (timing >= 100) {
      refPage.editNum(1, load_ref);
      refPage.pageDraw();

      timing = 0;
    }

    if (pressed) {
      pressed = false;
      return;
    }
  }
}

// MAIN --------------------------------------------------------------------------------

void setup() {

  Serial.begin(9600);
  menPage.begin();

  // Stepper setup
  stepper.setMaxSpeed(speed_max);
  pos_home = stepper.currentPosition();

  // PinModes for non-library used pins
  pinMode(PIN_POT, INPUT);
  pinMode(PIN_SWITCH, INPUT);
  pinMode(PIN_LED1, OUTPUT);

  // Loadcell setup
  cell.begin(PIN_DT, PIN_CLK);

  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), interruptStop, FALLING);
}

int selection = NAN;
void loop() {

  int selection_old = selection;
  selection = (analogRead(A0) / 1023.0) * (entrys_num);

  if (selection_old != selection) {
    menPage.editMsg(selection, "> " + entrys[selection].getName());
    menPage.pageDraw();
    menPage.editMsg(selection, entrys[selection].getName());
  }

  if (pressed) {
    entrys[selection].pageDraw();
    pressed = false;

    if (entrys[selection].getName() == refPage.getName()) {
      refMode();
      delay(100);
      selection = NAN;
    } else if (entrys[selection].getName() == runPage.getName()) {
      runMode();
      delay(100);
      selection = NAN;
    } else if (entrys[selection].getName() == calPage.getName()) {
      calibrate();
      delay(100);
      selection = NAN;
    }
  }
}
