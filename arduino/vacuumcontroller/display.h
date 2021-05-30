/*
 * oled  display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

namespace display {

extern void reset();
extern void setup();
extern void loop();
extern void printScreen();

} // namespace display

#endif
// not truncated
