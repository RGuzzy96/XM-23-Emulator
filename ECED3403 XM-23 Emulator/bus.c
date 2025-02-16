#include "bus.h"
#include "memory.h"

int bus(uint16_t address, uint8_t* value, int mode) {
	
	// validate the address range
	if (address > MEMORY_SIZE) {
		printf("Bus error: Address 0x%04X is out of range.\n", address);
		return -1;
	}

	if (mode == BUS_READ) {
		*value = readMemory(address);
	}
	else if (mode == BUS_WRITE) {
		writeMemory(address, *value);
	}
	else {
		printf("Bus error: Invalid mode %d for address 0x%04X.\n", mode, address);
		return -2;
	}

	return 0;
}