/*
 * Breathing Led.
 * Provides simple led on/off, blinking and breathing.
 * Best if led is connected to a PWM output.
 */

#ifndef BREATHING_LED_H
#define BREATHING_LED_H

#include <Arduino.h>

namespace breathingLed {

static const uint8_t kLedPin = LED_BUILTIN;

extern void setup();

extern void loop();

extern void on();

extern void off();

/*
   * Smoothly change LED brightness. Requires pwm.
   */

extern void breathe();

/*
   * blink led.
   * "duty" is a duty cycle from 0 to 256, where 0 makes the pin always LOW and 256 makes the pin always HIGH.
   * "periodMillis" is a period in milliseconds.
   */

extern void blink(uint16_t duty = 127, uint16_t periodMillis = 1000);

} // namespace breathingLed

#endif
// not truncated
