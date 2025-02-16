#include "execute_al.h"
#include "registers.h"
#include "bus.h"

// function to update status flags in PSW after an arithmetic/logic operation
static void updateFlags(uint16_t result, uint16_t src, uint16_t dst, int isByteMode) {
	// use 8-bit mask if byte mode, 16 bit if not (word mode)
	uint16_t mask = isByteMode ? 0xFF : 0xFFFF;

	// mask result based on mode
	result &= mask;

	// reset all PSW flags before setting new ones
	PSW &= ~(PSW_Z | PSW_N | PSW_C | PSW_V);

	// update zero (Z) flag if result is 0
	if (result == 0) SET_FLAG(PSW_Z);

	// set negative (N) flag if result is negative (MSB is 1)
	if (result & (isByteMode ? 0x80 : 0x8000)) SET_FLAG(PSW_N);

	// set carry (C) flag if unsigned borrow occured (src greater than what is in dst)
	if ((uint16_t)dst < (uint16_t)src) SET_FLAG(PSW_C);

	// convert values to signed
	int16_t signedDst = (int16_t)(dst & mask);
	int16_t signedSrc = (int16_t)(src & mask);
	int16_t signedResult = (int16_t)(result & mask);

	// set overvflow (V) flag if signed overflow occured (checking if dst and src have same signs and if the result has different sign than dst)
	if (((signedDst < 0) == (signedSrc < 0)) && ((signedDst < 0) != (signedResult < 0))) {
		SET_FLAG(PSW_V);
	}
}

// helper function to handle BCD correction for result from DADD
static uint16_t applyBCDAdjustment(uint16_t result, int isByteMode) {
	// example, DST = 0x19 (19 in BCD) + SRC = 0x07 (7 in BCD) -> RESULT = 0x20 | wrong! this is why we use this function to correct to get 0x26 (26 in BCD)
	
	uint16_t adjusted = result;
	uint8_t carry = 0;

	// check if byte or word
	if (isByteMode) {
		// BCD correct lower nibble
		if ((result & 0x0F) > 9) {
			adjusted += 0x06;
		}
		// correct upper nibble
		if ((result & 0xF0) > 9) {
			adjusted += 0x60;
			carry = 1;
		}
	}
	else {
		// correct lower byte
		if ((result & 0x000F) > 9) {
			adjusted += 0x0006;
		}
		if ((adjusted & 0x00F0) > 0x0090) {
			adjusted += 0x0060;
			carry = 1;
		}

		// correct upper byte
		if ((adjusted & 0x0F00) > 0x0900) {
			adjusted += 0x0600;
		}
		if ((adjusted & 0xF000) > 0x9000) {
			adjusted += 0x6000;
			carry = 1;
		}
	}

	// if we needed a carry for correction, set flag
	if (carry) SET_FLAG(PSW_C);

	// return the BCD corrected result
	return adjusted;
}

int executeAL(Instruction* instruction) {
	uint16_t dstValue, result;
	int16_t srcValue;
	int isByteMode = instruction->wb;
	int useConstant = instruction->rc;

	// skip either dst or src fetch if R/C is 1 and use constant instead

	// read destination register value
	if (!readFromRegister(instruction->operands[0], &dstValue)) {
		printf("Error reading destination register in %s instruction\n", instruction->mnemonic);
		return 2;
	}

	// check if we skip register read and use constant
	if (useConstant) {
		srcValue = constants[instruction->operands[1]];
	}
	else {
		// read source register value
		if (!readFromRegister(instruction->operands[1], &srcValue)) {
			printf("Error reading source register in %s instruction\n", instruction->mnemonic);
			return 2;
		}
	}

	// mask source value based on byte/word mode
	if (isByteMode) {
		srcValue = (int8_t)srcValue;
	}

	switch (instruction->opcode) {
	case 0x40: // ADD
	case 0x41: // ADDC (addition with carry)
		printf("Adding %d and %d\n", dstValue, srcValue);
		result = dstValue + srcValue + (instruction->opcode == 0x41 && PSW & PSW_C ? 1 : 0); // add DST and SRC, with carry if ADDC
		updateFlags(result, srcValue, dstValue, isByteMode); // update the PSW flags based on the operation result
		return writeToRegister(instruction->operands[0], result, isByteMode, 0); // write result in dst register

	case 0x42: // SUB
	case 0x43: // SUBC (subtraction with carry)
		result = dstValue + (~srcValue + 1) + (instruction->opcode == 0x43 && PSW & PSW_C ? 1 : 0); // subtract dst and src, with carry if SUBC
		updateFlags(result, srcValue, dstValue, isByteMode); // update the PSW flags based on the operation result
		return  writeToRegister(instruction->operands[0], result, isByteMode, 0); // write result in dst register		

	case 0x44: // DADD (decimal addition, see: https://www.ibm.com/docs/en/i/7.3?topic=concepts-arithmetic-operations#MCNPFAO__title__4)
		result = dstValue + srcValue + (PSW & PSW_C ? 1 : 0); // addition with carry
		result = applyBCDAdjustment(result, isByteMode); // correct the result for BCD
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	case 0x45: // CMP
		printf("Comparing %d and %d\n", dstValue, srcValue);
		result = dstValue + (~srcValue + 1); // subtract DST and SRC
		updateFlags(result, srcValue, dstValue, isByteMode); // update the PSW flags based on the operation result
		return 0;

	case 0x46: // XOR (see: https://www.geeksforgeeks.org/bitwise-operators-in-c-cpp/)
		result = dstValue ^ srcValue;
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	case 0x47: // AND
		result = dstValue & srcValue;
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	case 0x48: // OR
		result = dstValue | srcValue;
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	case 0x49: // BIT (bit test)
		result = dstValue & (1 << srcValue);
		updateFlags(result, srcValue, dstValue, isByteMode);
		return 0;

	case 0x4A: // BIC (bit clear)
		result = dstValue & ~(1 << srcValue);
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	case 0x4B: // BIS (bit set)
		result = dstValue | (1 << srcValue);
		updateFlags(result, srcValue, dstValue, isByteMode);
		return writeToRegister(instruction->operands[0], result, isByteMode, 0);

	default:
		return 1;
	}
}