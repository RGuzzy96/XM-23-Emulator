#include "execute_mem.h"
#include "registers.h"
#include "bus.h"
#include <stdbool.h>

// helper to encapsulate writing to simulated memory for ST/STR
static int handleMemoryWrite(uint16_t address, uint16_t value, int isByteMode, char* mnemonic) {
	// if byte mode, store only the LSB
	if (isByteMode) {
		// mask to get lsb
		uint8_t lsb = value & 0xFF;

		// use bus to write lsb to memory
		if (bus(address, &lsb, BUS_WRITE) != 0) {
			printf("Error writing byte to memory for %s instruction at 0x%04X\n", mnemonic, address);
			return 0;
		}
	}
	else {
		// shift and mask to separate lsb and msb
		uint8_t lsb = value & 0xFF;
		uint8_t msb = (value >> 8) & 0xFF;

		// use bus to write both lsb and msb to memory
		if (bus(address, &lsb, BUS_WRITE) != 0 || bus(address + 1, &msb, BUS_WRITE) != 0) {
			printf("Error writing word to memory for %s instruction at 0x%04X\n", mnemonic, address);
			return 0;
		}
	}

	// return 1 for success
	return 1;
}

// helper function to handle pre/post increment/decrement
static int adjustAddressWithPRPO(Instruction* instruction, uint16_t* address, uint8_t registerIdentifier, int checkPre) {
	bool check = false;
	bool didChange = false;

	// modify check for pre or post
	if (checkPre) {
		check = instruction->prpo == 1;
	}
	else {
		check = instruction->prpo == 0;
	}

	// if check condition satisfied, increment or decrement, or no action, based on extracted dec/inc bits
	if (check) {
		if (instruction->dec) {
			*address -= (instruction->wb ? 1 : 2);
			didChange = true;
		}
		else if (instruction->inc) {
			*address += (instruction->wb ? 1 : 2);
			didChange = true;
		}
	}

	// if address was incremented or decremented, update the address stored in the register
	if (didChange) {
		return writeToRegister(registerIdentifier, *address, 0, 0);
	}

	return 0;
}

int executeMEM(Instruction* instruction) {
	switch (instruction->opcode) {
	case 0x58: // LD, LDR
	case 0x80:
	{
		uint16_t addressFromSource;
		uint16_t valueFromMemory = 0;

		// get the value (address for desired byte or word) stored in the source register
		if (!readFromRegister(instruction->operands[1], &addressFromSource)) {
			printf("Error reading register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// add offset to address from source if LDR
		if (instruction->opcode == 0x80) {
			int16_t offset = instruction->operands[2] & 0x3F;  // extract lower 6 bits

			// if bit 5 is set, extend the sign to 16 bits
			if (instruction->operands[2] & 0x20) {
				offset |= 0xFFC0;
			}

			if (instruction->wb) {
				addressFromSource += offset;  // byte mode
			}
			else {
				addressFromSource += offset * 2;  // word mode
			}
		}

		// handle pre increment/decrement if needed
		if(instruction->opcode == 0x58 && adjustAddressWithPRPO(instruction, &addressFromSource, instruction->operands[1], 1) == 2) {
			printf("Error pre incrementing/decrementing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// fetch the lsb from memory
		uint8_t lsb;
		if (bus(addressFromSource, &lsb, 0) != 0) {
			printf("Error accessing bus for LSB in %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// if word mode, fetch the msb as well
		if (!instruction->wb) {
			uint8_t msb;
			if (bus(addressFromSource + 1, &msb, 0) != 0) {
				printf("Error accessing bus for MSB in %s instruction\n", instruction->mnemonic);
				return 2;
			}
			valueFromMemory = (msb << 8) | lsb;
		}
		else {
			valueFromMemory = lsb;
		}

		// write the value from source register in destination register
		if (writeToRegister(instruction->operands[0], valueFromMemory, instruction->wb, 0) == 2) {
			printf("Error writing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}
	
		// handle post increment/decrement if needed
		if (instruction->opcode == 0x58 && adjustAddressWithPRPO(instruction, &addressFromSource, instruction->operands[1], 0) == 2) {
			printf("Error post incrementing/decrementing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		return 0;
	}
	case 0x5C:
	case 0xC0: // ST, STR
	{
		uint16_t addressToWrite;
		uint16_t valueToStore;

		// get the value to store in memory from the source register
		if (!readFromRegister(instruction->operands[1], &valueToStore)) {
			printf("Error reading register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// get the memory address for writing from the destination register
		if (!readFromRegister(instruction->operands[0], &addressToWrite)) {
			printf("Error reading register for ST %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// handle pre increment/decrement if needed
		if (instruction->opcode == 0x5C && adjustAddressWithPRPO(instruction, &addressToWrite, instruction->operands[0], 1) == 2) {
			printf("Error pre incrementing/decrementing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// if STR, add the offset
		if (instruction->opcode == 0xC0) {
			int16_t offset = instruction->operands[2] & 0x3F; // extract lower 6 bits

			// if bit 5 is set, extend the sign
			if (instruction->operands[2] & 0x20) {
				offset |= 0xFFC0;  // extend sign
			}

			if (instruction->wb) {
				addressToWrite += offset;  // byte mode
			}
			else {
				addressToWrite += offset * 2;  // word mode
			}
		}
		
		// write the register value to memory
		if (!handleMemoryWrite(addressToWrite, valueToStore, instruction->wb, instruction->mnemonic)) return 2;

		// handle post increment/decrement if needed
		if (instruction->opcode == 0x5C && adjustAddressWithPRPO(instruction, &addressToWrite, instruction->operands[0], 0) == 2) {
			printf("Error pre incrementing/decrementing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}

		return 0;
	}
	default:
		return 1;
	}
}