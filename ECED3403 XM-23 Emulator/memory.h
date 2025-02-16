#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 65536 // 64kB memory
#define MAX_MEMORY_PRINT 48 // define max memory addresses to print at a time (keeping small to not overwhelm the console)

extern uint8_t *memory;

// initializes simulated memory for the XM-23 program
void initializeMemory();

// frees the simulated memory
void cleanupMemory();

// reads and returns single byte from memory at the provided address
uint8_t readMemory(uint16_t address);

// writes single byte value provided to memory at the provided address
void writeMemory(uint16_t address, uint8_t value);

// writes an array of data to memory contiguously, starting at the provided start address
void writeArrayToMemory(uint16_t startAddress, uint8_t* data, int dataLength);

// prints a specified section of memory to the console
void printMemorySection(uint16_t startingAddress, int length);

#endif // !MEMORY_H

