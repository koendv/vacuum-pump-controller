/*
 * watchdog timer
 */

#include "watchdog.h"
#include <IWatchdog.h>

namespace watchdog {

void setup() {
  if (IWatchdog.isReset(true)) {
    Serial.println();
    Serial.println("watchdog reboot");
  }
  IWatchdog.begin(10000000); // 10 seconds watchdog timer
}
void loop() {
  IWatchdog.reload();
}
void reboot() {
  NVIC_SystemReset();
}

} // namespace watchdog

// not truncated
