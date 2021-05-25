#include "settings.h"
#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_MAGIC_NUMBER 0xdeadbeef

namespace settings {

double Kp, Ki, Kd, setpoint;
bool logging;

/*
   * settings as saved in non-volatile
   */

struct eepromSettings {
  uint32_t magicNumber;
  double Kp, Ki, Kd, setpoint;
  bool logging;
};

/*
   * save current settings to non-volatile
   */

void write() {
  struct eepromSettings conf;
  conf.Kp = Kp;
  conf.Ki = Ki;
  conf.Kd = Kd;
  conf.setpoint = setpoint;
  conf.logging = logging;
  conf.magicNumber = EEPROM_MAGIC_NUMBER;
  EEPROM.put(0, conf);
  return;
}

/*
   * read current settings from non-volatile
   */

void read() {
  struct eepromSettings conf;
  EEPROM.get(0, conf);
  if (conf.magicNumber != EEPROM_MAGIC_NUMBER) {
    // corrupted or uninitialized
    conf.Kp = kDefaultKp;
    conf.Ki = kDefaultKi;
    conf.Kd = kDefaultKd;
    conf.setpoint = kDefaultSetpoint;
    conf.logging = kDefaultLogging;
    conf.magicNumber = EEPROM_MAGIC_NUMBER;
    EEPROM.put(0, conf);
    Serial.println("init");
  }
  Kp = conf.Kp;
  Ki = conf.Ki;
  Kd = conf.Kd;
  setpoint = conf.setpoint;
  logging = conf.logging;
  return;
}

void setup() {
  read(); // restore last saved config
}

} // namespace settings

// not truncated
