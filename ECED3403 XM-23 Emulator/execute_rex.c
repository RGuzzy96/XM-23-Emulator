#include "execute_rex.h"
#include "registers.h"

int executeREX(Instruction* instruction) {
	int isByteMode = instruction->wb;
	uint16_t srcValue, dstValue;

	// read source register value
	if (!readFromRegister(instruction->operands[1], &srcValue)) {
		printf("Error reading source register in %s instruction\n", instruction->mnemonic);
		return 2;
	}

	srcValue &= (isByteMode ? 0xFF : 0xFFFF);

	switch (instruction->opcode) {
	case 0x4C00: // MOV
		// write the whole word to the destination register
		return writeToRegister(instruction->operands[0], srcValue, isByteMode, 0);
	case 0x4C80: // SWAP
		// read destination register value
		if (!readFromRegister(instruction->operands[0], &dstValue)) {
			printf("Error reading destination register in %s instruction\n", instruction->mnemonic);
			return 2;
		}

		// write src value in dst register, write dst value in src register
		if (writeToRegister(instruction->operands[0], srcValue, isByteMode, 0) == 2 || writeToRegister(instruction->operands[1], dstValue, isByteMode, 0) == 2) {
			printf("Error writing register for %s instruction\n", instruction->mnemonic);
			return 2;
		}
		return 0;
	default:
		return 1;
	}
}