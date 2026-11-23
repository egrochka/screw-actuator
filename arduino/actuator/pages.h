#include <LiquidCrystal.h>
#include <functional>
#include "config.h"

LiquidCrystal lcd(PIN_DISP_RS, PIN_DISP_EN, PIN_DISP_DT4, PIN_DISP_DT5, PIN_DISP_DT6, PIN_DISP_DT7);

class Page {

private:

  static int const _chars = 20;
  static int const _rows = 4;

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

  std::function<void(Page&)> callback;

  String _name;
  float _nums[_rows];
  String _msgs[_rows];

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
             refPage._name,
             runPage._name,
             calPage._name);