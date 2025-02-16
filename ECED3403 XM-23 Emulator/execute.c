#include "execute.h"
#include "execute_rin.h"
#include "execute_toc.h"
#include "execute_mem.h"
#include "execute_al.h"
#include "execute_rex.h"
#include "execute_so.h"
#include "registers.h"

#include <stdio.h>
#include <stdlib.h>

char* getErrMsg(int errCode) {
	switch (errCode) {
		case 1:
			return "Unknown instruction";
		case 2:
			return "Error occured during instruction execution";
		default:
			return "Unexpected execution error";
	}
}

// helper function to organize the printing structure for a decoded instruction based on type
static const PrintInfo* getPrintInfo(Instruction* instruction) {
	static const PrintInfo printInfo1 = {
		{ "Destination", "Source" }, 
		{ "R%d", "R%d" }
	};

	static const PrintInfo printInfo2 = {
		{ "Destination", "Value" },
		{ "R%d", "0x%02x" }
	};

	static const PrintInfo printInfo3 = {
		{ "Branch PC", "Current PC", "Encoded Offset"},
		{ "0x%04x", "0x%04x", "0x%04x"}
	};

	static const PrintInfo printInfo4 = {
		{ "Destination" },
		{ "R%d" }
	};

	static PrintInfo printInfoModified;

	switch (instruction->type) {
		case AL:
			printInfoModified = printInfo1;
			// if set for byte, adjust dst display
			if (instruction->wb) {	
				printInfoModified.formats[0] = "A%d";
				printInfoModified.formats[1] = "A%d";
			}

			// if using constant, remove annotation for source register
			if (instruction->rc) {
				printInfoModified.formats[1] = "%d";
			}
			return &printInfoModified;
		case REX:
			return &printInfo1;
		case MEM:
			printInfoModified = printInfo1;
			// if 3 operands, add the offset as an operand to print (for LDR and STR)
			if (instruction->operandCount == 3) {				
				printInfoModified.labels[2] = "Offset";
				printInfoModified.formats[2] = "0x%04x";
			}
			// if byte mode, adjust register displays
			if (instruction->wb == 1) {
				// for load ops, dst in byte mode, for store ops, src in byte mode
				if (instruction->mnemonic == "LD" || instruction->mnemonic == "LDR") {
					printInfoModified.formats[0] = "A%d";
				}
				else {
					printInfoModified.formats[1] = "A%d";
				}
			}
			return &printInfoModified;
		case RIN:
			return &printInfo2;
		case TOC:
			return &printInfo3;
		case SO:
			// if set for byte, adjust dst display
			if (instruction->wb == 1) {
				printInfoModified = printInfo4;
				printInfoModified.formats[0] = "A%d";
				return &printInfoModified;
			}
			return &printInfo4;
		default:
			return NULL;
	}
}

static void printInstructionDetails(Instruction* instruction) {
	printf("Instruction: %s%s\n", instruction->mnemonic, instruction->wb == 1 ? ".B" : "");
	
	if (instruction->rc == 0 || instruction->rc == 1) {
		printf("R/C: %d\n", instruction->rc);
	}

	if (instruction->wb == 0 || instruction->wb == 1) {
		printf("W/B: %d\n", instruction->wb);
	}

	// get labels for operands based on instruction type
	const PrintInfo* printInfo = getPrintInfo(instruction);

	// loop through operands, backwards since we put them in the array destination-first
	for (int i = instruction->operandCount - 1; i >= 0; i--) {
		printf("%s: ", printInfo->labels[i]);
		printf(printInfo->formats[i], (instruction->rc && printInfo->labels[i] == "Source" ? constants[instruction->operands[i]] : instruction->operands[i]));
		printf("\n");
	}
}

int execute(Instruction* instruction) {
	int code = 0;

	// print instruction details before executing
	printInstructionDetails(instruction);

	// if not in the single operand or register exchange instruction classes, shift the opcode to just get a byte
	if (instruction->type != SO && instruction->type != REX) {
		instruction->opcode = instruction->opcode >> 8 & 0xFF;
	}

	// execute based on instruction type
	switch (instruction->type) {
		case RIN:
			code = executeRIN(instruction);
			break;
		case MEM:
			code = executeMEM(instruction);
			break;
		case TOC:
			code = executeTOC(instruction);
			break;
		case AL:
			code = executeAL(instruction);
			break;
		case REX:
			code = executeREX(instruction);
			break;
		case SO:
			code = executeSO(instruction);
			break;
		default:
			code = 1;
			break;
	}

	// return execution code
	return code;
}
