#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

uint8_t* memory = NULL;

void initializeMemory() {
	memory = (uint8_t*)calloc(MEMORY_SIZE, sizeof(uint8_t));
	if (memory == NULL) {
		printf("Failed to allocate memory");
		exit(1);
	}
}

void cleanupMemory() {
	if (memory != NULL) {
		free(memory);
		memory = NULL;
	}
}

uint8_t readMemory(uint16_t address) {
	// check if address is within range
	if (address >= MEMORY_SIZE) {
		printf("Error: Attempt to read from address 0x%04X, beyond limit 0x%04X\n", address, MEMORY_SIZE);
		return 0;
	}
	// return byte at memory address
	return memory[address];
}

void printMemorySection(uint16_t startingAddress, int length) {

	// valid start address within range
	if (startingAddress >= MEMORY_SIZE) {
		printf("Error: Attempt to print from address 0x%04x, beyond limit 0x%04x\n", startingAddress, MEMORY_SIZE);
		return;
	}

	// validate memory display length, trim if necessary
	if (length > MAX_MEMORY_PRINT) {
		printf("Warning: Limiting memory display to %d bytes.\n", MAX_MEMORY_PRINT);
		length = MAX_MEMORY_PRINT;
	}

	printf("Address      Value\n");
	printf("------------------\n");
	for (int i = 0; i < length; i++) {
		// valid each address within range before print
		if (startingAddress + i >= MEMORY_SIZE) {
			printf("Error: Attempt to print from address 0x%04x, beyond limit 0x%04x\n", startingAddress + 1, MEMORY_SIZE);
			return;
		}
		// print memory from valid address
		printf("0x%04X       0x%02X\n", startingAddress + i, readMemory(startingAddress + i));
	}
}

void writeMemory(uint16_t address, uint8_t value) {
	// check if memory is within range
	if (address >= MEMORY_SIZE) {
		printf("Error: Attempt to write to address 0x%04X, beyond limit 0x%04X\n", address, MEMORY_SIZE);
		return;
	}
	// write value to memory
	memory[address] = value;
}

void writeArrayToMemory(uint16_t startAddress, uint8_t *data, int dataLength) {
	// loop through specified data length, write byte to the current address
	for (int i = 0; i < dataLength; i++) {
		// check if memory is within range
		if (startAddress + i >= MEMORY_SIZE) {
			printf("Error: Attempt to write to address 0x%04X, beyond limit 0x%04X\n", startAddress + i, MEMORY_SIZE);
			return;
		}

		memory[startAddress + i] = data[i];
	}	
}