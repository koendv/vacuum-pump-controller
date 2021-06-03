/*
 * calculate free memory. stm32duino specific.
 */

/*
 * After MemoryAllocationStatistics.ino by Frederic Pillon.
 * This code is in the public domain.
 */

#include "bytesfree.h"
#include <malloc.h>

extern "C" char *sbrk(int i);
/* externs depend upon linker script. e.g. STM32F103XB_FLASH.ld */
extern char _estack;
extern char _Min_Stack_Size;
static char *ramend = &_estack;
static char *min_stack_ptr = (char *)(ramend - &_Min_Stack_Size);

/* calculate free memory
 *  free memory is sum of:
 *  - memory between stack pointer and end of heap (stack_ptr - heapend)
 *  - memory on heap but not allocated yet (mi.fordblks)
 */

size_t bytes_free() {
  char *heapend = (char *)sbrk(0);
  char *stack_ptr = (char *)__get_MSP();
  struct mallinfo mi = mallinfo();
  size_t bytesfree = ((stack_ptr < min_stack_ptr) ? stack_ptr : min_stack_ptr) - heapend + mi.fordblks;
  return bytesfree;
}

// not truncated
