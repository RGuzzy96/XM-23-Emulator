#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// define cpu clock
extern uint32_t cpuClock;

// function to start and control the fetch/decode/execute loop
void cpuCycle();

// initializes the global program counter to the provided address
void initializePC(uint16_t address);

#endif // !CPU_H

