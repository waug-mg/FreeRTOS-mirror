#include "bsp.h"

// to communicate with the debugger
volatile uint64_t tohost __attribute__((aligned(64)));
volatile uint64_t fromhost __attribute__((aligned(64)));

// Define a temporary external interrupt handler
void external_interrupt_handler( uint32_t cause ) {
    (void) cause;
}
