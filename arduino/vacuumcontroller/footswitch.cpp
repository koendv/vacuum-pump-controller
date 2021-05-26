#include "footswitch.h"
#include "motor.h"

// footswitch pins. footswitch has two contacts.
// one contact is normally open, one contact is normally closed.
#define PIN_FTSW_NO PC15
#define PIN_FTSW_NC PC14

namespace footswitch {
static uint32_t lastTimeMillis = 0;
const uint32_t sampleTimeMillis = 5;
enum state_enum { FTSW_INIT,
                  FTSW_ON,
                  FTSW_OFF };
state_enum ftsw_state;

void setup() {
  pinMode(PIN_FTSW_NO, INPUT_PULLUP);
  pinMode(PIN_FTSW_NC, INPUT_PULLUP);
  ftsw_state = FTSW_INIT;
  lastTimeMillis = millis();
  return;
}

void loop() {
  uint32_t nowMillis = millis();
  if ((nowMillis < lastTimeMillis) || (nowMillis >= lastTimeMillis + sampleTimeMillis)) {
    bool ftsw_no = digitalRead(PIN_FTSW_NO); // footswitch contact normally open
    bool ftsw_nc = digitalRead(PIN_FTSW_NC); // footswitch contact normally closed
    bool ftsw_on = ftsw_no && !ftsw_nc;
    bool ftsw_off = !ftsw_no && ftsw_nc;
    switch (ftsw_state) {
    case FTSW_INIT:
      if (ftsw_off) {
        Serial.println("footswitch");
        motor::setswitch(2, false);
        ftsw_state = FTSW_OFF;
      }
      break;
    case FTSW_ON:
      if (ftsw_off) {
        // Serial.println("footswitch off");
        motor::setswitch(2, false);
        ftsw_state = FTSW_OFF;
      }
      break;
    case FTSW_OFF:
      if (ftsw_on) {
        // Serial.println("footswitch on");
        motor::setswitch(2, true);
        ftsw_state = FTSW_ON;
      }
      break;
    default:
      ftsw_state = FTSW_INIT;
      break;
    }
    lastTimeMillis = nowMillis;
  }
}
} // namespace footswitch
// not truncated
