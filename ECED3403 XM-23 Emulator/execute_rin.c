#include "execute_rin.h"
#include "registers.h"

int executeRIN(Instruction* instruction) {
	switch (instruction->opcode) {
		case 0x60: // MOVL
			// write the immediate byte value to the LSB of the destination register
			return writeToRegister(instruction->operands[0], instruction->operands[1], 1, 0);
		case 0x68: // MOVLZ
		{
			// set the MSB to 0x00 (0) and the LSB to the immediate byte value
			uint16_t wordForRegister = (0x00 << 8) | (instruction->operands[1]);

			// write the whole word to the destination register
			return writeToRegister(instruction->operands[0], wordForRegister, 0, 0);
		}
		case 0x70: // MOVLS
		{
			// set the MSB to 0xFF (-1) and the LSB to the immediate byte value
			uint16_t wordForRegister = (0xFF << 8) | (instruction->operands[1]);

			// write the whole word to the destination register
			return writeToRegister(instruction->operands[0], wordForRegister, 0, 0);
		}
		case 0x78: // MOVH
			// write the immediate byte value to the MSB of the destination register
			return writeToRegister(instruction->operands[0], instruction->operands[1], 1, 1);
		default:
			return 1;
	}
}