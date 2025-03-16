#include "decode.h"
#include "fetch.h"
#include "cpu.h"
#include "registers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// function to extract the operands and bit flags from instruction words based on opcode type
static void extractOperandsAndFlags(uint16_t instructionWord, Instruction* instruction) {
	
	// define bool variables for shared properties between instruction types
	bool hasDestination = 
		instruction->type == MEM
		|| instruction->type == AL
		|| instruction->type == RIN
		|| instruction->type == REX
		|| instruction->type == SO;

	bool hasSource =
		instruction->type == MEM
		|| instruction->type == AL
		|| instruction->type == REX;

	bool hasRC = instruction->type == AL;

	bool hasWB =
		instruction->type == MEM
		|| instruction->type == AL
		|| instruction->type == REX
		|| (instruction->type == SO && instruction->mnemonic != "SWPB" && instruction->mnemonic != "SXT");

	// extract the destination operand, if any
	if (hasDestination) {
		// mask to just get last 3 bits
		instruction->operands[0] = instructionWord & 0x07;
	}

	// extract the source operand, if any
	if (hasSource) {
		// shift and mask to get bits 5-3
		instruction->operands[1] = (instructionWord >> 3) & 0x07;
	}
	else if (instruction->type == RIN) {
		// shift and mask to get the 8-bit value for register-initialization instruction
		instruction->operands[1] = (instructionWord >> 3) & 0xFF;
	}

	// extract the value for W/B, if any
	if (hasWB) {
		// shift and mask to get bit 6
		instruction->wb = (instructionWord >> 6) & 0x01;
	}

	// extract the value for R/C, if any
	if (hasRC) {
		// shift and mask to get bit 7
		instruction->rc = (instructionWord >> 7) & 0x01;
	}

	// if transfer of control, extract offset and get PC values
	if (instruction->type == TOC) {
		// extract offset
		if (instruction->mnemonic == "BL") {
			instruction->operands[2] = instructionWord & 0x1FFF;

			// sign-extend if needed
			if (instruction->operands[2] & 0x1000) {
				instruction->operands[2] |= 0xE000;
			}
		}
		else {
			instruction->operands[2] = instructionWord & 0x03FF;

			// sign-extend if needed
			if (instruction->operands[2] & 0x0200) {
				instruction->operands[2] |= 0xFC00;
			}
		}
		
		// load current PC
		instruction->operands[1] = registerFile[R_PC];

		instruction->operands[0] = registerFile[R_PC] + ((int16_t)instruction->operands[2] * 2); // multiplying offset by 2 because it is a word offset, not byte offset
	}

	// if LD or ST, extract pre or post increment or decrement
	if (instruction->mnemonic == "LD" || instruction->mnemonic == "ST") {
		instruction->inc = (instructionWord >> 7) & 0x01;
		instruction->dec = (instructionWord >> 8) & 0x01;
		instruction->prpo = (instructionWord >> 9) & 0x01;
	}

	// if LDR or STR, extract encoded offset for source address
	if (instruction->mnemonic == "LDR" || instruction->mnemonic == "STR") {
		// mask to get offset in bits 13-7, store in operands
		instruction->operands[2] = (instructionWord >> 8) & 0x007F;
	}
}

static int loadOpcodeInfo(Instruction* instruction) {
	// get length of table
	size_t tableLength = sizeof(opcodeTable) / sizeof(opcodeTable[0]);

	// find opcode in opcode table
	for (size_t i = 0; i < tableLength; i++) {
		// use mask and attempt to match opcode with table entry
		uint16_t maskedOpcode = (instruction->opcode & opcodeTable[i].opcodeMask);
		if (maskedOpcode == opcodeTable[i].opcode) {
			// load opcode info into instruction struct
			instruction->mnemonic = opcodeTable[i].mnemonic;
			instruction->operandCount = opcodeTable[i].operandCount;
			instruction->type = opcodeTable[i].type;
			instruction->rc = -1;
			instruction->wb = -1;

			// replace unmasked opcode with masked one for future use
			instruction->opcode = maskedOpcode;

			return 1;
		}
	}

	// return 0 if operand not found in table
	return 0;
}

int decode(uint16_t instructionWord, Instruction* instruction) {

	// set opcode as instruction word for now, will shift and mask as needed later
	instruction->opcode = instructionWord;

	// load opcode info from opcode table into instruction
	if (!loadOpcodeInfo(instruction)) {
		return 0;
	}

	// valid opcode, now extract the operands and any applicable bit flags
	extractOperandsAndFlags(instructionWord, instruction);

	return 1;
}
