#include "loopstats.h"

namespace loopstats {
static uint32_t lastTimeMillis = UINT32_MAX;
static uint32_t slowestLoopMillis = 0;

uint32_t slowest_loop() {
  uint32_t slowest = slowestLoopMillis;
  slowestLoopMillis = 0;
  return slowest;
}

void loop() {
  uint32_t nowMillis = millis();
  if (nowMillis >= lastTimeMillis) {
    uint32_t loopMillis = nowMillis - lastTimeMillis;
    if (loopMillis > slowestLoopMillis)
      slowestLoopMillis = loopMillis;
    lastTimeMillis = nowMillis;
  } else { // overflow
    lastTimeMillis = nowMillis;
  }
}
} // namespace loopstats
// not truncated
