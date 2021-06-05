/*
  * vacuum pump controller
  *
  * compiled for blue pill.
  *
  * Requirements:
  * - Adafruit_SSD1306 library
  * - Adafruit_GFX library
  * - Adafruit_BMP280 library
  * - PID library v1.2.0 by Brett Beauregard
  * - STM32RTC library
  *
  * Compilation settings:
  * - Board: Generic STM32F1 Series
  * - Board part number: Bluepill F103CB (or C8 with 128k)
  * - USART support: enabled (generic 'Serial')
  * - USB support: CDC (generic 'Serial' supersede U(S)ART)
  * - USB speed: Low/Full Speed
  * - Optimize: Smallest (-Os)
  * - C Runtime library: newlib nano
  */

// clang-format -style="{ColumnLimit: 0}"

// build-opt -DUSB_PRODUCT_STRING=\"vacuum\ pump\"

#include "breathing_led.h"
#include "console.h"
#include "display.h"
#include "footswitch.h"
#include "loopstats.h"
#include "motor.h"
#include "pidctrl.h"
#include "sensor.h"
#include "settings.h"
#include "uptime.h"
#include "watchdog.h"

const int sample_time_millis = 20;

void setup() {
  console::setup(); // open usb serial
  watchdog::setup();
  uptime::setup();
  breathingLed::setup();
  settings::setup();
  display::setup();
  sensor::setup();
  motor::setup();
  footswitch::setup();
  PIDctrl::setup();
  sensor::setSampleTime(sample_time_millis);
  PIDctrl::setSampleTime(sample_time_millis);

  Serial.println("ready");
  Serial.print('>');
}

void loop() {
  console::loop();
  watchdog::loop();
  breathingLed::loop();
  display::loop();
  sensor::loop();
  PIDctrl::loop();
  footswitch::loop();
  loopstats::loop();
}

// not truncated
