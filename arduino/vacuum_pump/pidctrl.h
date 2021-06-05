#ifndef PIDCTRL_H
#define PIDCTRL_H

#include <Arduino.h>

namespace PIDctrl {

extern double motorPWM;
extern void setup();
extern void loop();
extern void reload();                  // reload Kp, Ki, Kd
extern void reset();                   // reload Kp, Ki, Kd and reset controller
extern void manual(double pwm);        // put controller in manual mode
extern void automatic();               // put controller in automatic mode
extern bool isAuto();                  // true if PID controller running in automatic
extern void setSampleTime(int millis); // set time between output updates in milliseconds

} // namespace PIDctrl
#endif

// not truncated
