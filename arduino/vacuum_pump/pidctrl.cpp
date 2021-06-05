// Proportional on Measurement PID controller
// from https://github.com/br3ttb/Arduino-PID-Library
// http://brettbeauregard.com/blog/2017/06/introducing-proportional-on-measurement/

#include "pidctrl.h"
#include "breathing_led.h"
#include "motor.h"
#include "sensor.h"
#include "settings.h"
#include <PID_v1.h>

namespace PIDctrl {

double motorPWM = MINPWM;

PID PIDcontrol(&sensor::vacuum, &motorPWM, &settings::setpoint, settings::Kp, settings::Ki, settings::Kd, P_ON_M, DIRECT);

void setup() {
  PIDcontrol.SetTunings(settings::Kp, settings::Ki, settings::Kd);
  PIDcontrol.SetOutputLimits(MINPWM, MAXPWM);
  PIDcontrol.SetSampleTime(sensor::kSampleMillis);
  PIDcontrol.SetMode(AUTOMATIC);
}

void reload() {
  PIDcontrol.SetTunings(settings::Kp, settings::Ki, settings::Kd);
}

void reset() {
  PIDcontrol.SetMode(MANUAL);
  setup(); // pid controller is reset when switching from manual to automatic
}

// set motor speed manually
void manual(double pwm) {
  PIDcontrol.SetMode(MANUAL);
  motor::speed(pwm);
  motorPWM = pwm;
}

// auto motor speed control
void automatic() {
  PIDcontrol.SetMode(AUTOMATIC);
}

bool isAuto() {
  return PIDcontrol.GetMode() == AUTOMATIC;
}

void setSampleTime(int millis) {
  PIDcontrol.SetSampleTime(millis);
}

void loop() {
  if (isnan(sensor::vacuum))
    return;
  PIDcontrol.Compute();
  motor::speed(motorPWM);
}

} // namespace PIDctrl

// not truncated
