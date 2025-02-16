#include "registers.h"

uint16_t registerFile[REGISTER_COUNT] = {
	0x0000,	// 0
	0x0001,	// 1
	0x0002,	// 2
	0x0004,	// 4
	0x0008,	// 8
	0x0010,	// 16
	0x0020,	// 32
	0xFFFF	// -1
};

uint16_t PSW = 0x0000; // initialize PSW with no flags set 

void initializeRegisterFile() {
	// explicitly reset values to constant values
	registerFile[0] = 0x0000;
	registerFile[1] = 0x0001;
	registerFile[2] = 0x0002;
	registerFile[3] = 0x0004;
	registerFile[4] = 0x0008;
	registerFile[5] = 0x0010;
	registerFile[6] = 0x0020;
	registerFile[7] = 0xFFFF;
}

int writeToRegister(uint8_t registerIdentifier, uint16_t value, int isByteMode, int isMSB) {
	// make sure we are going to write to a valid register
	if (registerIdentifier >= REGISTER_COUNT || registerIdentifier < 0) {
		printf("Register Write Error: Attempting to write to non-existing register\n");
		return 2;
	}

	// check word or byte mode
	if (isByteMode) {
		// check if we want to write to MSB or LSB
		if (isMSB) {
			// mask to maintain LSB and replace MSB with value byte
			registerFile[registerIdentifier] = (registerFile[registerIdentifier] & 0x00FF) | (value << 8);
		}
		else {
			// mask to maintain MSB and replace LSB with value byte
			registerFile[registerIdentifier] = (registerFile[registerIdentifier] & 0xFF00) | (value & 0xFF);
		}
	}
	else {
		// write whole word to register
		registerFile[registerIdentifier] = value;
	}

	return 0;
}

int readFromRegister(uint8_t registerIdentifier, uint16_t* value) {
	// make sure we are going to read from a valid register
	if (registerIdentifier >= REGISTER_COUNT || registerIdentifier < 0) {
		printf("Register Read Error: Attempting to read from non-existing register\n");
		return 0;
	}

	// get value from register file at the provided index
	*value = registerFile[registerIdentifier];

	return 1;
}

void displayRegisterFile() {
	// print header
	printf("Register         Value\n");
	printf("---------------------------\n\n");

	// loop through registers in register file
	for (int i = 0; i < REGISTER_COUNT; i++) {
		printf("R%d               0x%04x\n", i, registerFile[i]);
	}

	// print extra newline for spacing
	printf("\n");
}

void displayPSW() {
	// print header
	printf("Program Status Word: 0x%04x\n", PSW);

	// print binary form
	printf("Binary: ");
	// loop through each bit
	for (int i = 15; i >= 0; i--) {
		printf("%d", (PSW >> i) & 0x01);
		// add space every 4 bits for readability 
		if (i % 4 == 0) printf(" ");
	}
	printf("\n");

	// decode and print individual fields using proper masks
	printf("Decoded Fields:\n");
	printf("  Previous Priority (PP)  : %d\n", (PSW & PSW_PP_MASK) >> 13);
	printf("  Fault (FLT)             : %d\n", (PSW & PSW_FLT_MASK) >> 8);
	printf("  Current Priority (CP)   : %d\n", (PSW & PSW_CP_MASK) >> 5);
	printf("  Overflow (V)            : %d\n", (PSW & PSW_V_MASK) >> 4);
	printf("  Sleep (SLP)             : %d\n", (PSW & PSW_SLP_MASK) >> 3);
	printf("  Negative (N)            : %d\n", (PSW & PSW_N_MASK) >> 2);
	printf("  Zero (Z)                : %d\n", (PSW & PSW_Z_MASK) >> 1);
	printf("  Carry (C)               : %d\n", (PSW & PSW_C_MASK));

	// extra newline for spacing
	printf("\n");
}
