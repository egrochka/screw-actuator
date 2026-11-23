#ifndef CONFIG_H
#define CONFIG_H

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
  
  int weight_cal = 109;  // Calibration load weight in grams

  // Various speeds
  int speed_min = 0;
  int speed_max = 5000;
  int speed_main = 0;
  int speed_return = -3000;

#endif