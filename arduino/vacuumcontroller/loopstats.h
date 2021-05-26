#ifndef LOOPSTATS_H
#define LOOPSTATS_H 1
/* 
 *  slowest loop in milliseconds. Used to tune program performance.
 */
#include <Arduino.h>

namespace loopstats {
extern uint32_t slowest_loop();
void loop();
} // namespace loopstats
#endif
// not truncated
