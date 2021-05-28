#include "uptime.h"
#include <Arduino.h>
#include <STM32RTC.h>

namespace uptime {
STM32RTC &rtc = STM32RTC::getInstance();

void setup() {
  rtc.begin(true);
}

// print uptime
void print() {
  uint32_t days = rtc.getDay() - 1; // max. 30d
  uint32_t hours = rtc.getHours();
  uint32_t mins = rtc.getMinutes();
  Serial.print("up ");
  if (days > 0) {
    Serial.print(days);
    Serial.print(" d ");
  }
  Serial.print(hours);
  Serial.print(':');
  if (mins < 10)
    Serial.print('0');
  Serial.println(mins);
}
} // namespace uptime
// not truncated
