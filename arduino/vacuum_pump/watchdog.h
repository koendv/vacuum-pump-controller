/*
 * watchdog timer
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>

namespace watchdog {

extern void setup();  // call once, at boot
extern void loop();   // call frequently
extern void reboot(); // reboots system

} // namespace watchdog

#endif
// not truncated
