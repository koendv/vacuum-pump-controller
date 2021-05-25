#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

// number of sensors
#define NUM_SENSOR 4

namespace sensor {

const int kSampleMillis = 200;
enum state_enum { NO_SENSOR,
                  SENSOR_OK,
                  SENSOR_ERR };
extern state_enum state[NUM_SENSOR];

extern double vacuum;
extern int ipressure[NUM_SENSOR];    // pressure in Pa
extern double fpressure[NUM_SENSOR]; // pressure in hPa
extern void readSensors();
extern void printSensors();

extern void setSampleTime(int millis);
extern void setup();
extern void loop();

} // namespace sensor
#endif

// not truncated
