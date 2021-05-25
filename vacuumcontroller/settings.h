#ifndef SETTINGS_H
#define SETTINGS_H

namespace settings {

static const double kDefaultKp = 150.0;
static const double kDefaultKi = 50.0;
static const double kDefaultKd = 0.0;
static const double kDefaultSetpoint = 100.0;
static const bool kDefaultLogging = false;

extern double Kp; // proportional
extern double Ki; // integral
extern double Kd; // differential
extern double setpoint;
extern bool logging;

void write(); // writes current config to non-volatile storage.
void read();  // reads current config to non-volatile storage.

extern void setup();
} // namespace settings

#endif

// not truncated
