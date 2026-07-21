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
bool state = 0;  // 0 - STOP, 1-RUN
float pos_home;
int weight_cal = 109;  //1000;  // Calibration load weight in grams
int lcd_rows = 4;
int lcd_char = 20;

// Various speeds
int speed_min = 0;
int speed_max = 8000;
int speed_main = 0;
int speed_return = 3000;

// Create objects from libraries
AccelStepper stepper;
HX711 cell;
LiquidCrystal lcd(PIN_DISP_RS, PIN_DISP_EN, PIN_DISP_DT4, PIN_DISP_DT5, PIN_DISP_DT6, PIN_DISP_DT7);

class Screen {
  // Show information on the 4x20 display

private:
  int _pin_dt;
  int _pin_clk;
  int _rows;
  int _chars;
  String _mode;

  void deletefrom(String msg, int row) {
    int len = msg.length();
    int chars_max = _chars;
    if (row >= 2) {
      len += 16;
      chars_max += 16;
    }
    for (int i = len; i < chars_max; i++) {
      lcd.setCursor(i, row);
      lcd.print(" ");
    }
  }

public:
  Screen() {}  //DO NOT USE

  Screen(int pin_dt, int pin_clk, int rows, int chars) {
    _rows = rows;
    _chars = chars;
    _mode = "NONE";
  }

  void begin() {
    lcd.begin(_rows, _chars);
    delay(1000);
    lcd.clear();
  }

  void show(String mode, int num1 = 0, int num2 = 0) {
    if (mode == "REF") {  //REFERENCE MODE
      if (_mode != mode) {
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 0);
        lcd.print("Reference mode");
      }

      String msg = "Reference: ";

      deletefrom(msg, 1);
      lcd.setCursor(0, 1);
      lcd.print(msg + String(num1));

    } else if (mode == "READ") {  //READING MODE
      if (_mode != mode) {
        lcd.setCursor(0, 0);
        lcd.print("                    ");
        lcd.setCursor(0, 0);
        lcd.print("Reading mode");
      }

      String msg1 = "Reading: ";
      deletefrom(msg1, 2);
      lcd.setCursor(16, 2);
      lcd.print(msg1 + String(num1));

      String msg2 = "Current speed: ";
      deletefrom(msg2, 3);
      lcd.setCursor(16, 3);
      lcd.print(msg2 + String(num2));

    } else if (mode == "CAL") {  //CALIBRATION MODE
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Calibration mode");
      lcd.setCursor(0, 1);

      switch (int(num1)) {
        case 0:
          lcd.print("                    ");
          lcd.setCursor(0, 1);
          lcd.print("Tare");
          break;

        case 1:
          lcd.print("                    ");
          lcd.setCursor(0, 1);
          lcd.print("Hang weight");
          break;

        case 2:
          lcd.print("                    ");
          lcd.setCursor(0, 1);
          lcd.print("Done!");
          break;
      }
    }
    _mode = mode;
  }
};

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
class Switch {

private:
  int _pin;
  bool _state;
  bool _stateOLD;

public:
  Switch(){};  //DO NOT USE

  Switch(int pin) {
    _pin = pin;
  }
  bool read() {
    bool temp = digitalRead(_pin);
    return temp;
  }
  bool ispressed() {
    _stateOLD = _state;
    _state = read();
    if (_stateOLD && !_state) {
      delay(10);
      return true;
    } else {
      delay(10);
      return false;
    }
  }
};

class MenuEntry {

public:
  int _number;
  String _name;
  String _msg;

  MenuEntry() {}  // DO NOT USE

  MenuEntry(int number, String name) {
    _number = number;
    _name = name;
  }

  String name() {
    return _name;
  }
};

class Controller {
  // Controller parameters

private:
  float Kp = 35;
  float Ki = 0;
  float Kd = 1.0;  // 0.25;
  float integral = 0;

public:

  int _update_time = 45;

  Controller() {}

  float PID(int load_ref, int load_in, int load_old, long dt) {

    int err = (load_ref - load_in);
    float P = Kp * err;

    integral += err * dt;
    float I = Ki * constrain(integral, load_min, load_max);

    int dErr = (load_in - load_old) / dt;
    float D = Kd * dErr;

    float C = P + I + D;
    C = constrain(C, -load_ref, load_ref);
    int speed = map(C, load_min, load_ref, speed_min, speed_max);

    //Serial.println("P: " + String(P) + " I: " + String(I) + " D: " + String(D) + " C: " + String(C) + " Speed: " + String(speed));
    //Serial.println(String(load_in) + " " + String(millis()) + " " + String(load_ref) + " " + String(speed));
    return speed;
  }

  float reset() {
    integral = 0;
  }
};

// Create objects from classes
Screen screen(PIN_DT, PIN_SCK, lcd_rows, lcd_char);
Controller controller;
LED LED1(PIN_LED1);
Switch switch1(PIN_SWITCH);

float calibrate() {
  // Calibrate loadcell before use

  screen.show("CAL", 0);
  cell.tare(20);
  float offset = cell.get_offset();

  Serial.println("Press");

  screen.show("CAL", 1);
  while (!switch1.ispressed())
    ;
  cell.calibrate_scale(weight_cal, 5);

  Serial.println("OK");

  screen.show("CAL", 2);
  return offset;
}

float offset;

bool stop = false;
void interruptStop() {
  stop = true;
  delay(300);
}

void setup() {

  Serial.begin(9600);
  screen.begin();

  // Stepper setup
  stepper.setMaxSpeed(speed_max);
  pos_home = stepper.currentPosition();

  // PinModes for non-library used pins
  pinMode(PIN_POT, INPUT);
  pinMode(PIN_SWITCH, INPUT);
  pinMode(PIN_LED1, OUTPUT);

  // Loadcell setup
  cell.begin(PIN_DT, PIN_CLK);
  offset = calibrate();

  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), interruptStop, FALLING);
}

// Variables stored between loops
float load_in = cell.get_units();
float load_ref;
bool switch_in = false;
long time_current = millis();

void runMode() {

  int timing = 0;

  LED1.off();

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

      screen.show("READ", load_in, speed_main);

      timing = 0;
    }

    stepper.runSpeed();

    if (stop) break;
  }
}

void refMode() {

  int timing = 0;

  LED1.on();
  controller.reset();

  speed_main = speed_min;
  stepper.moveTo(pos_home);
  stepper.setSpeed(speed_return);

  while (true) {

    Serial.println(String(pos_home) + " dsdsad " + String(stepper.currentPosition()));
    stepper.moveTo(pos_home);
    stepper.runSpeed();

    long time_old = time_current;
    time_current = millis();
    long dt = time_current - time_old;
    timing += dt;

    load_ref = (analogRead(PIN_POT) / 1023.0) * load_max;

    if (timing >= 100) {
      screen.show("REF", load_ref);
      timing = 0;
    }

    if (stop) break;
  }
}

void loop() {

  stop = false;

  switch (state) {

    case 0:
      // User can input load on potentiometer

      refMode();
      state = 1;
      break;

    case 1:
      // Actuator is run towards the reference load

      runMode();
      state = 0;
      break;
  }
}
