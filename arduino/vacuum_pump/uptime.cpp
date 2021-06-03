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
  uint8_t days = rtc.getDay() - 1; // wraps after 30 days
  uint8_t hours, minutes;
  rtc.getTime(&hours, &minutes, NULL, NULL, NULL);
  Serial.print("up ");
  if (days > 0) {
    Serial.print(days);
    Serial.print(" d ");
  }
  Serial.print(hours);
  Serial.print(':');
  if (minutes < 10)
    Serial.print('0');
  Serial.println(minutes);
}
} // namespace uptime
// not truncated
