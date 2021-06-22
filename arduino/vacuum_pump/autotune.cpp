#include "autotune.h"
#include "breathing_led.h"
#include "display.h"
#include "motor.h"
#include "pidctrl.h"
#include "sensor.h"
#include "settings.h"
#include "watchdog.h"
#include <Arduino.h>

namespace autotune {

/* relay test to determine Kp, Ki, Kd
   *
   * Procedure:
   * - open exhaust valve
   * - switch off pump
   * - wait until all air escapes
   * - close exhaust valve
   * - starting from pwm 0, increase pump pwm voltage until pressure increases. This gives minimum pwm to get pump running.
   * - set pump at full speed (100% duty cycle) until pressure no longer increases. This gives steady-state pressure.
   * - open exhaust valve, switch off pump, wait until all air escapes
   * - close exhaust valve
   * - set pump at full speed (100% duty cycle) until pressure equal or bigger than 0.63 * steady-state pressure
   * - stop pump
   * - time constant tau is time needed to get to 0.63 * steady-state pressure
   * - repeat 5 times and average to reduce measuring errors
   * - calculate Kp, Ki, Kd.
   */

void openValve() {
  motor::setswitch(2, HIGH);
}

void closeValve() {
  motor::setswitch(2, LOW); // valve normally closed
}

// print pressure and motor pwm on console; also reset watchdog during autotune.
void printStatus() {
  int pwm = motor::pwma_percent + 0.5;
  int pressure = sensor::vacuum + 0.5;
  if (pwm < 100)
    Serial.print(' ');
  if (pwm < 10)
    Serial.print(' ');
  Serial.print(pwm);
  Serial.print("% ");
  if (pressure < 1000)
    Serial.print(' ');
  if (pressure < 100)
    Serial.print(' ');
  if (pressure < 10)
    Serial.print(' ');
  Serial.print(pressure);
  Serial.print(" hPa\r");
  breathingLed::loop();
  display::loop();
  watchdog::loop();
}

void tune() {
  const int kCycles = 5;          // average over 5 runs
  const double kMeasureErr = 0.5; // measuring error, hPa
  const int maxCount = 1000;      // timeout in 0.1s

  int minpwm, count; // loop variables

  double minPressure, maxPressure, tauPressure;
  uint32_t beginMillis, endMillis, tauMillis, steadyPressureCount;
  double tau, tau1, Kp, sumKp, Kp1, Ki, sumKi, Ki1;
  const double lambda = 3.0; // tuning factor. increase if unstable.

  Serial.println("autotune - press c to cancel");
  sumKp = 0;
  sumKi = 0;

  for (int cycle = 0; cycle <= kCycles; cycle++) {
    Serial.print("cycle ");
    Serial.print(cycle);
    Serial.print('/');
    Serial.println(kCycles);

    // stop pump, open exhaust valve
    motor::speed(MINPWM);
    openValve();

    // wait until pressure zero
    for (count = 0; count < maxCount; count++) {
      sensor::readSensors();
      printStatus();
      if (sensor::vacuum < kMeasureErr)
        break;
      if (Serial.available() && (Serial.read() == 'c'))
        return;
      delay(100);
    }

    if (count == maxCount) {
      // valve open but pressure not zero
      Serial.println("check sensor, valve");
      return;
    }
    Serial.println();

    // close exhaust valve. increase pump speed until pressure begins to increase
    closeValve();
    for (minpwm = 0; minpwm < MAXPWM; minpwm++) {
      motor::speed(minpwm);
      sensor::readSensors();
      printStatus();
      if (sensor::vacuum > 2 * kMeasureErr)
        break;
      if (Serial.available() && (Serial.read() == 'c'))
        return;
      delay(5);
    }

    if (minpwm == MAXPWM) {
      // pump at max speed but pressure does not increase
      Serial.println("check cables and tubes");
      return;
    }
    Serial.println();

    // exhaust valve closed, pump at max speed, measure steady-state pressure
    const uint32_t sample_delay1 = 500; // milliseconds
    steadyPressureCount = 0;
    maxPressure = 0;
    motor::speed(MAXPWM);
    for (count = 0; count < maxCount; count++) {
      sensor::readSensors();
      printStatus();
      if (sensor::vacuum > maxPressure) {
        maxPressure = sensor::vacuum;
        steadyPressureCount = 0;
      } else {
        steadyPressureCount++;
        if (steadyPressureCount > 10000 / sample_delay1)
          break; // 10s of unchanged max. vacuum
      }
      if (Serial.available() && (Serial.read() == 'c'))
        return;
      delay(sample_delay1);
    }
    Serial.println();
    if (maxPressure == 0) {
      // pump running but maximum pressure zero
      Serial.println("check pump, sensor");
      return;
    }

    // stop pump, open exhaust valve
    motor::speed(MINPWM);
    openValve();

    // wait until pressure zero
    for (count = 0; count < maxCount; count++) {
      sensor::readSensors();
      printStatus();
      if (sensor::vacuum < kMeasureErr)
        break;
      if (Serial.available() && (Serial.read() == 'c'))
        return;
      delay(100);
    }

    if (count == maxCount) {
      // valve open but pressure not zero
      Serial.println("check sensor, valve");
      return;
    }
    Serial.println();

    // exhaust valve closed, pump at max speed, measure time to 0.63 times steady-state pressure
    motor::speed(MAXPWM);
    closeValve();

    minPressure = sensor::vacuum;
    tauPressure = minPressure + 0.63212 * (maxPressure - minPressure); // 0.63212 = 1-exp(-1)
    tauMillis = 0;
    beginMillis = millis();
    for (count = 0; count < maxCount; count++) {
      sensor::readSensors();
      printStatus();
      if (sensor::vacuum >= tauPressure)
        break;
      if (Serial.available() && (Serial.read() == 'c'))
        return;
      delay(100);
    }
    tauMillis = millis();
    // switch pump off
    motor::speed(MINPWM);

    if (count == maxCount) {
      // pump running but pressure does not reach setpoint
      Serial.println("check cables and tubes");
      return;
    }
    Serial.println();

    // lambda rule
    tau = (tauMillis - beginMillis) / 1000.0;
    Kp1 = (MAXPWM - minpwm) / (maxPressure - minPressure) / lambda;

    if ((tau <= 0) || (Kp1 <= 0)) {
      Serial.println("error");
      return;
    }

    Ki1 = Kp1 / tau;

    Serial.print("minpwm: ");
    Serial.print(minpwm);
    Serial.print(" maxpwm: ");
    Serial.print(MAXPWM);
    Serial.print(" minpressure: ");
    Serial.print(minPressure);
    Serial.print(" maxpressure: ");
    Serial.println(maxPressure);
    Serial.print("tau: ");
    Serial.print(tau);
    Serial.print("s Kp: ");
    Serial.print(Kp1);
    Serial.print(" Ki: ");
    Serial.println(Ki1);

    // don't use data from first cycle
    // first cycle is used to get system in steady state
    if (cycle != 0) {
      sumKp += Kp1;
      sumKi += Ki1;
    }
  }

  // average to reduce noise
  Kp = sumKp / float(kCycles);
  Ki = sumKi / float(kCycles);

  settings::Kp = Kp;
  settings::Ki = Ki;
  settings::Kd = 0.0;
  PIDctrl::reload();

  Serial.print("Kp: ");
  Serial.print(settings::Kp);
  Serial.print(" Ki: ");
  Serial.print(settings::Ki);
  Serial.print(" Kd: ");
  Serial.println(settings::Kd);
  Serial.println("enter w to save");
}

} // namespace autotune
