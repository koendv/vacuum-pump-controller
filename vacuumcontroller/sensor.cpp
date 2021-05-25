#include "sensor.h"
#include "motor.h"
#include "settings.h"
#include <math.h>

#include "breathing_led.h"

// uncomment one and only one of these two includes, to compile for BMP280 or BME280
#include <Adafruit_BMP280.h>
//#include <Adafruit_BME280.h>

//#define SIMULATION

// Chip selects for the four BMP280 headers
#define H1_CS PB12
#define H2_CS PA8
#define H3_CS PB11
#define H4_CS PB10

// pins for SPI bus 1
#define SCK1 PA5
#define MOSI1 PA7
#define MISO1 PA6

// pins for SPI bus 2
#define SCK2 PB13
#define MOSI2 PB15
#define MISO2 PB14

// pins for SPI bus 3
#define SCK3 PB3
#define MOSI3 PB5
#define MISO3 PB4

namespace sensor {
double vacuum;
uint32_t lastMillis = 0;
uint32_t sampleTime = 100;
int ipressure[NUM_SENSOR];
double fpressure[NUM_SENSOR];
const int8_t cs[NUM_SENSOR] = {H1_CS, H2_CS, H3_CS, H4_CS};
uint8_t sensor_id[NUM_SENSOR] = {0};

SPIClass SPITwo(MOSI2, MISO2, SCK2);
//SPIClass SPIThree(MOSI3, MISO3, SCK3);      // hardware SPI3 on STM32F411

// array of sensor chips
#ifdef __BMP280_H__
Adafruit_BMP280 sensor[NUM_SENSOR]{
    Adafruit_BMP280(H1_CS, &SPITwo),            // hardware SPI2
    Adafruit_BMP280(H2_CS, &SPITwo),            // hardware SPI2
    Adafruit_BMP280(H3_CS, MOSI3, MISO3, SCK3), // software SPI3 on STM32F103
    Adafruit_BMP280(H4_CS, MOSI3, MISO3, SCK3)  // software SPI3 on STM32F103
};
#endif

#ifdef __BME280_H__
Adafruit_BME280 sensor[NUM_SENSOR]{
    Adafruit_BME280(H1_CS, &SPITwo),            // hardware SPI2
    Adafruit_BME280(H2_CS, &SPITwo),            // hardware SPI2
    Adafruit_BME280(H3_CS, MOSI3, MISO3, SCK3), // software SPI3 on STM32F103
    Adafruit_BME280(H4_CS, MOSI3, MISO3, SCK3)  // software SPI3 on STM32F103
};
#endif

/*
   * Sensor states:
   * NO_SENSOR  no sensor detected
   * SENSOR_OK  sensor detected and running
   * SENSOR_ERR sensor not responsive.
   *
   * Detect sensor by switching sensor chip select to INPUT_PULLDOWN;
   * CS will be high if sensor present.
   *
   * Detect sensor hung or removed by reading sensorID().
   *
   */

state_enum state[NUM_SENSOR];

void setSampleTime(uint32_t millis) {
  if (millis > 0)
    sampleTime = millis;
}

// logging.
// logging is the only line of output that begins with a tab '\t'
// logs time, motor pwm, pressure in Pa, and checksum.
// checksum is simple 32-bit sum of timestamp, pressures and motor pwm.

void logSensors(uint32_t now) {
  uint32_t checksum = 0;
  Serial.print('\t');
  Serial.print(now);
  checksum = now;
  Serial.print('\t');
  Serial.print(motor::pwma);
  checksum += motor::pwma;
  Serial.print('\t');
  for (int i = 0; i < NUM_SENSOR; i++) {
    Serial.print(ipressure[i]);
    Serial.print('\t');
    checksum += ipressure[i];
  }
  Serial.print(checksum);
  Serial.print('\t');
  Serial.println();
}

void printSensors() {
  const static char *msg[] = {"- ", "ok ", "? "};
  Serial.print("sensors ");
  for (int i = 0; i < NUM_SENSOR; i++) {
    uint8_t s = state[i];
    if ((s < 0) || (s > SENSOR_ERR))
      s = SENSOR_ERR;
    Serial.print(msg[s]);
    Serial.print(' ');
  }
  Serial.println();
}

void detectSensor(uint8_t i) {
  state[i] = NO_SENSOR;
  ipressure[i] = 0;
  fpressure[i] = 0.0;
  // detect sensor by reading cs pin - sensor pulls cs up
  pinMode(cs[i], INPUT_PULLDOWN);
  delay(10); // allow cs to settle
  bool detected = digitalRead(cs[i]);
  if (!detected)
    return;
  if (sensor[i].begin()) {
    sensor_id[i] = sensor[i].sensorID();
    state[i] = SENSOR_OK;
  } else { // unknown sensor type
    pinMode(cs[i], INPUT_PULLDOWN);
    breathingLed::blink(); // blink rapidly
    state[i] = SENSOR_ERR;
  }
}

void detectSensors() {
  for (int i = 0; i < NUM_SENSOR; i++) {
    detectSensor(i);
  }
}

#ifdef SIMULATION

/* simulate sensor and vacuum vessel */
void readSensors() {
  static uint32_t lastSensorMillis = 0;
  uint32_t now = millis();
  float delta_t = (now - lastSensorMillis) / 1000;
  lastSensorMillis = now;

  double motor = 0.0, valve = 0.0;
  const double deadZone = MAXPWM / 5.0; // pump does not turn if voltage too low
  if (motor::pwma > deadZone)
    motor = (motor::pwma - deadZone) / (MAXPWM - deadZone);
  valve = motor::getswitch(2);
  double steadystate = motor * 1000 - valve * 500;
  if (steadystate < 0.0)
    steadystate = 0.0;

  for (int i = 0; i < NUM_SENSOR; i++)
    pressure[i] = 0.0;
  vacuum = vacuum + (steadystate - vacuum) / 40.0;
}

#else

void readSensor(int i) {
  sensors_event_t pressure_event;
  bool detected;
  int count;
  switch (state[i]) {
  case NO_SENSOR:
    // check if sensor plugged in
    // assume pinMode(cs[i], INPUT_PULLDOWN)
    detected = digitalRead(cs[i]);
    if (detected) {
      detectSensor(i);
      if (state[i] != NO_SENSOR)
        printSensors();
    }
    break;
  case SENSOR_OK:
    // read pressure
    if (sensor[i].sensorID() == sensor_id[i]) {
      // getPressureSensor has a resolution of 1 hPa
      // readPressure has a resolution of 0.01 hPa, so we'll use that - smoother regulation
      // sensor accuracy is 0.12 hPa
      ipressure[i] = sensor[i].readPressure();
      fpressure[i] = ipressure[i] / 100.0; // in hPa
    } else {
      ipressure[i] = 0;
      fpressure[i] = 0;
      breathingLed::blink(); // blink rapidly
      pinMode(cs[i], INPUT_PULLDOWN);
      state[i] = SENSOR_ERR;
      printSensors();
    }
    break;
  case SENSOR_ERR:
    // wait until sensor unplugged
    if (digitalRead(cs[i]))
      break;
    state[i] = NO_SENSOR;
    // stop blinking rapidly if no sensors in error state
    count = 0;
    for (int i = 0; i < NUM_SENSOR; i++) {
      if (state[i] == SENSOR_ERR)
        count++;
    }
    if (count == 0)
      breathingLed::blink(224, 5000); // stop blinking rapidly
    printSensors();
    break;

  default:
    // should never happen
    Serial.print("state? ");
    Serial.println(state[i]);
    pinMode(cs[i], INPUT_PULLDOWN);
    state[i] = NO_SENSOR;
    break;
  }
}

void readSensors() {
  for (int i = 0; i < NUM_SENSOR; i++)
    readSensor(i);
  // fpressure[0]] = atmospheric, fpressure[1] = vacuum vessel
  vacuum = fpressure[0] - fpressure[1];
}
#endif

void setup() {
  detectSensors();
  printSensors();
}

void loop() {
  uint32_t now = millis();
  if ((now < lastMillis) || (now - lastMillis > sampleTime)) {
    readSensors();
    if (settings::logging)
      logSensors(now);
    lastMillis = now;
  }
}
} // namespace sensor

// not truncated
