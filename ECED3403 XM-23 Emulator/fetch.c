#include "fetch.h"
#include "bus.h"
#include "registers.h"

#include <stdio.h>

uint16_t fetch() {
	// define the high and low bytes of the instruction word that will be fetched
	uint8_t highByte, lowByte;
	printf("Fetching from address 0x%04X\n", registerFile[R_PC]);

	// fetch the low byte of the instruction from memory
	if (bus(registerFile[R_PC], &lowByte, 0) != 0) {
		printf("Bus error during fetch at address 0x%04X\n", registerFile[R_PC]);
		exit(1);
	}
	registerFile[R_PC]++; // increment PC

	// fetch the high byte of the instruction from memory
	if (bus(registerFile[R_PC], &highByte, 0) != 0) {
		printf("Bus error during fetch at address 0x%04X\n", registerFile[R_PC]);
		exit(1);
	}
	registerFile[R_PC]++; // increment PC again

	// merge the two bytes into a single word to return
	return (highByte << 8) | lowByte;
}