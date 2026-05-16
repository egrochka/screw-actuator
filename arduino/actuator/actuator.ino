// CONTROL NEMA23 STEPPER MOTOR USING A LOADCELL INPUT

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <TM1637Display.h>
#include <HX711.h>

// Define pins for input/output
#define PIN_CW1 2
#define PIN_CW2 3
#define PIN_CCW1 4
#define PIN_CCW2 5
#define PIN_LED1 0
#define PIN_LED2 1
#define PIN_SWITCH 6
#define PIN_CLK 10
#define PIN_DIO 11
#define PIN_SCK 12
#define PIN_DT 13
#define PIN_POT A0

// Define other variables
int load_min = 0;
int load_max = 8000;
bool state = 0; // 0 - STOP, 1-RUN
float pos_home;
int weight_cal = 1000; // Calibration load weight in grams

// Various speeds
int speed_min = 0;
int speed_max = 1000;
int speed_main = 0;
int speed_return = 50;

// Create objects from libraries
AccelStepper stepper;
TM1637Display display(PIN_CLK, PIN_DIO);
HX711 cell;

class Screen {
  // Show information on the TM1637 display

  private:
    int _pin_dt;
    int _pin_clk;

  public:
    Screen() {} //DO NOT USE

    Screen(int pin_dt, int pin_clk) {
      _pin_dt = pin_dt;
      _pin_clk = pin_clk;
    }
    
    void show(float num) {
      display.clear();
      display.showNumberDecEx(num);
    }
};

class Controller {
  // Controller parameters

  private:
    float Kp = 1;
    float Ki = 1;
    float Kd = 1;
    float integral = 1;

  public:
    Controller() {}

    float PID(int load_ref, int load_in, int load_old, long dt) {

      int err = (load_ref - load_in);
      float P = Kp * err;

      integral += err*dt;
      float I = Ki * constrain(integral, load_min/Ki, load_max/Ki);

      int dErr = (load_in - load_old)/dt;
      float D = Kd * dErr;

      float C = P + I + D;

      int speed = map(C, load_min, load_ref, speed_min, speed_max);
      return speed;
    }

    float reset() {
      integral = 0;
    }
};

// Create objects from classes
Screen screen(PIN_DT, PIN_SCK);
Controller controller;

float calibrate() {
  // Calibrate loadcell before use

  screen.show(0);
  while(!digitalRead(PIN_SWITCH)) delay(1000);
  cell.tare(20);
  float offset = cell.get_offset();

  screen.show(1);
  while(!digitalRead(PIN_SWITCH)) delay(1000);
  cell.calibrate_scale(weight_cal, 5);

  screen.show(2);
  return offset; 
}

// Variables stored between loops
int load_in = cell.get_units();
int load_ref;
bool switch_in = false;
long time_current = millis();
float offset;

void setup() {

  // Stepper setup
  stepper.setMaxSpeed(speed_max);
  pos_home = stepper.currentPosition();

  // PinModes for non-library used pins
  pinMode(PIN_POT, INPUT);
  pinMode(PIN_SWITCH, INPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  // Loadcell setup
  cell.begin(PIN_DT, PIN_SCK);
  offset = calibrate();
}

void loop() {

  bool switch_old = switch_in;
  switch_in = digitalRead(PIN_SWITCH); 
  if (!switch_old && switch_in) state = !state, delay(1000); 
  
  long time_old = time_current;
  time_current = millis();
  long dt = time_current - time_old;

  while(!cell.is_ready());

  switch(state) {

    case 0:
      // User can input load on potentiometer

      digitalWrite(PIN_LED1, HIGH);
      digitalWrite(PIN_LED2, LOW);

      load_ref = analogRead(PIN_POT) * load_max / 5;
      screen.show(load_ref);

      speed_main = speed_min;
      stepper.moveTo(pos_home);
      stepper.setSpeed(speed_return);
      stepper.runSpeedToPosition();

      controller.reset();

      break;

    case 1:
      // Actuator is run towards the reference load

      digitalWrite(PIN_LED1, LOW);
      digitalWrite(PIN_LED2, HIGH);

      float load_old = load_in;
      load_in = cell.get_units();
      speed_main = controller.PID(load_ref, load_in, load_old, dt);

      screen.show(load_in);

      stepper.setSpeed(speed_main);
      stepper.runSpeed();

      break;
  }
}
