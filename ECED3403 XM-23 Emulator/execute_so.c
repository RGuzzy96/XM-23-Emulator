#include "execute_so.h"
#include "registers.h"

int executeSO(Instruction* instruction) {
	uint16_t dstValue, result;
	int isByteMode = instruction->wb;

	// read destination register value
	if (!readFromRegister(instruction->operands[0], &dstValue)) {
		printf("Error reading destination register in %s instruction\n", instruction->mnemonic);
		return 2;
	}

	switch (instruction->opcode) {
		case 0x4D00: // SRA
			result = (int16_t)dstValue >> 1; // perform arithmetic right shift, preserve sign
			updateFlags(result, 0, dstValue, isByteMode, 0); // update psw flags based on operation
			return writeToRegister(instruction->operands[0], result, isByteMode, 0); // write shifted result to dst register
		case 0x4D08: // RRC
		{
			uint16_t carry = (PSW & PSW_C) ? 0x8000 : 0x0000; // extract carry flag
			result = (dstValue >> 1) | carry; // perform right rotate through carry
			
			// update carry flag with LSB of original value
			if (dstValue & 0x1) SET_FLAG(PSW_C);
			else CLEAR_FLAG(PSW_C);
			updateFlags(result, 0, dstValue, isByteMode, 0); // update psw flags based on operation
			return writeToRegister(instruction->operands[0], result, isByteMode, 0); // write rotated result to dst register
		}
		case 0x4D10: // COMP
			result = ~dstValue; // flip all bits
			updateFlags(result, 0, dstValue, isByteMode, 0); // update psw flags based on operation
			return writeToRegister(instruction->operands[0], result, isByteMode, 0); // write flipped-bit result to dst register
		case 0x4D18: // SWPB
			result = ((dstValue & 0x00FF) << 8) | ((dstValue & 0xFF00) >> 8); // swap lower and upper bytes
			updateFlags(result, 0, dstValue, 0, 0); // update psw flags based on operation
			return writeToRegister(instruction->operands[0], result, 0, 0); // write swapped result to dst register
		case 0x4D20: // SXT
			result = (int16_t)(int8_t)(dstValue & 0xFF); // extend the sign bit of the lower byte to the full 16-bit word
			updateFlags(result, 0, dstValue, 0, 0); // update psw flags based on operation
			return writeToRegister(instruction->operands[0], result, 0, 0); // write sign-extended result to dst register
		default:
			return 1;
	}
}