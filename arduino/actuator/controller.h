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