#include "motor.h"
#include <Arduino.h>

#define TB6612_PWMA PB6
#define TB6612_AIN2 PA2
#define TB6612_AIN1 PA1
#define TB6612_STBY PA0
#define TB6612_PWMB PB7
#define TB6612_BIN1 PB8
#define TB6612_BIN2 PB9

namespace motor {

int pwma, pwmb;
float pwma_percent, pwmb_percent;

void setup() {

  analogWriteFrequency(20000);
  analogWriteResolution(16);

  pinMode(TB6612_STBY, OUTPUT);
  pinMode(TB6612_PWMA, OUTPUT);
  pinMode(TB6612_AIN1, OUTPUT);
  pinMode(TB6612_AIN2, OUTPUT);
  pinMode(TB6612_PWMB, OUTPUT);
  pinMode(TB6612_BIN1, OUTPUT);
  pinMode(TB6612_BIN2, OUTPUT);

  digitalWrite(TB6612_STBY, LOW);
  analogWrite(TB6612_PWMA, MAXPWM);
  digitalWrite(TB6612_AIN1, LOW);
  digitalWrite(TB6612_AIN2, LOW);
  analogWrite(TB6612_PWMB, MAXPWM);
  digitalWrite(TB6612_BIN1, LOW);
  digitalWrite(TB6612_BIN2, LOW);
  digitalWrite(TB6612_STBY, HIGH);

  pwma = 0;
  pwmb = 0;
  pwma_percent = 0;
  pwmb_percent = 0;
}

// use up to two brushless DC motors

void speed(int pwm, int port) {
  if (isnan(pwm))
    return;
  if (pwm < MINPWM)
    pwm = MINPWM;
  if (pwm > MAXPWM)
    pwm = MAXPWM;

  if (port == 0) { // port A
    pwma = pwm;
    pwma_percent = pwma * 100.0 / MAXPWM;
    if (pwm == 0) {
      digitalWrite(TB6612_AIN1, LOW);
      digitalWrite(TB6612_AIN2, LOW);
      analogWrite(TB6612_PWMA, pwm);
    } else if (pwm > 0) {
      digitalWrite(TB6612_AIN1, HIGH);
      digitalWrite(TB6612_AIN2, LOW);
      analogWrite(TB6612_PWMA, pwm);
    } else {
      digitalWrite(TB6612_AIN1, LOW);
      digitalWrite(TB6612_AIN2, HIGH);
      analogWrite(TB6612_PWMA, -pwm);
    }
  } else { // port B
    pwmb = pwm;
    pwmb_percent = pwmb * 100.0 / MAXPWM;
    if (pwm == 0) {
      digitalWrite(TB6612_BIN1, LOW);
      digitalWrite(TB6612_BIN2, LOW);
      analogWrite(TB6612_PWMB, pwm);
    } else if (pwm > 0) {
      digitalWrite(TB6612_BIN1, HIGH);
      digitalWrite(TB6612_BIN2, LOW);
      analogWrite(TB6612_PWMB, pwm);
    } else {
      digitalWrite(TB6612_BIN1, LOW);
      digitalWrite(TB6612_BIN2, HIGH);
      analogWrite(TB6612_PWMB, -pwm);
    }
  }
}

// use up to four on/off switches

void setswitch(int nr, bool onoff) {
  switch (nr) {
  case 0:
    digitalWrite(TB6612_PWMA, HIGH);
    digitalWrite(TB6612_AIN1, onoff);
    break;
  case 1:
    digitalWrite(TB6612_PWMA, HIGH);
    digitalWrite(TB6612_AIN2, onoff);
    break;
  case 2:
    digitalWrite(TB6612_PWMB, HIGH);
    digitalWrite(TB6612_BIN1, onoff);
    break;
  case 3:
    digitalWrite(TB6612_PWMB, HIGH);
    digitalWrite(TB6612_BIN2, onoff);
    break;
  default:
    break;
  }
}

bool getswitch(int nr) {
  bool retval = false;
  switch (nr) {
  case 0:
    retval = digitalRead(TB6612_AIN1);
    break;
  case 1:
    retval = digitalRead(TB6612_AIN2);
    break;
  case 2:
    retval = digitalRead(TB6612_BIN1);
    break;
  case 3:
    retval = digitalRead(TB6612_BIN2);
    break;
  default:
    break;
  }
  return retval;
}
} // namespace motor

// not truncated
