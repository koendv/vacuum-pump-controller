#include "footswitch.h"
#include "motor.h"

// footswitch connector contacts
#define PIN_FTSW_TIP PA3
#define PIN_FTSW_R1 PC15
#define PIN_FTSW_R2 PC14

namespace footswitch {
static uint32_t lastTimeMillis = 0;
const uint32_t sampleTimeMillis = 10;
enum state_enum { FTSW_INIT,
                  FTSW_ON,
                  FTSW_OFF };
state_enum ftsw_state;

void setup() {
  pinMode(PIN_FTSW_TIP, INPUT_PULLUP);
  pinMode(PIN_FTSW_R1, INPUT_PULLUP);
  pinMode(PIN_FTSW_R2, INPUT_PULLUP);
  ftsw_state = FTSW_INIT;
  lastTimeMillis = millis();
  return;
}

void loop() {
  uint32_t nowMillis = millis();
  if ((nowMillis < lastTimeMillis) || (nowMillis >= lastTimeMillis + sampleTimeMillis)) {
    // footswitch pins. footswitch has two contacts.
    // one contact is normally open, one contact is normally closed.
    bool ftsw_no = digitalRead(PIN_FTSW_TIP); // footswitch contact normally open
    bool ftsw_nc = digitalRead(PIN_FTSW_R1);  // footswitch contact normally closed
    // contact debouncing
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
