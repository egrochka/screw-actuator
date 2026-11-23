// CONTROL NEMA23 STEPPER MOTOR USING A LOADCELL INPUT

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <HX711.h>
#include <LiquidCrystal.h>
#include <string>
#include "pages.h"
#include "config.h"
#include "controller.h"

// CLASSES ------------------------------------------------------------------------

AccelStepper stepper;
HX711 cell;
Controller controller;

// FUNCTIONS ------------------------------------------------------------------------

bool pressed = false;
void interruptStop() {
  pressed = true;
  delay(300);
}

void calibrate(Page& calPage) {
  // Calibrate loadcell before use

  calPage.pageDraw();
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

float load_in = cell.get_units();
float load_ref;
bool switch_in = false;
long time_current = millis();
float pos_home;

void runMode(Page& runPage) {

  runPage.pageDraw();

  int timing = 0;

  digitalWrite(PIN_LED1, HIGH);

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

      digitalWrite(PIN_LED1, LOW);
      return;
    }
  }
}

void refMode(Page& refPage) {

  refPage.pageDraw();

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

int const entrys_num = 3;
Page entrys[entrys_num] = {refPage, runPage, calPage};

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

  refPage.callback = refMode;
  runPage.callback = runMode;
  calPage.callback = calibrate;
}

int selection = NAN;
void loop() {

  int selection_old = selection;
  selection = (analogRead(A0) / 1023.0) * (entrys_num);

  if (selection_old != selection) {
    menPage.editMsg(selection, "> " + entrys[selection]._name);
    menPage.pageDraw();
    menPage.editMsg(selection, entrys[selection]._name);
  }

  if (pressed) {
    pressed = false;
    entrys[selection].callback(entrys[selection]);

  }
}