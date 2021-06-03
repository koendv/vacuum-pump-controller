#ifndef MOTOR_H
#define MOTOR_H

#define MAXPWM 65535
#define MINPWM 0

namespace motor {

extern int pwma, pwmb;
extern float pwma_percent, pwmb_percent;
extern void setup();
extern void speed(int pwm, int port = 0);  // use up to two brushless DC motors
extern void setswitch(int nr, bool onoff); // use up to four on/off switches
extern bool getswitch(int nr);

} // namespace motor
#endif

// not truncated
