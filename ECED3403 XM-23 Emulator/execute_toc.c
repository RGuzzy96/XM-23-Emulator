#include "execute_toc.h"
#include "cpu.h"
#include "registers.h"

int executeTOC(Instruction* instruction) {
	switch (instruction->opcode) {
	case 0x00: // BL
		// set LR (R5) return address to current PC
		registerFile[R_LR] = registerFile[R_PC];
		
		// set PC to link address (branch PC)
		registerFile[R_PC] = instruction->operands[0];
		return 0;

	case 0x20: // BEQ/BZ
		// check if zero flag is up on PSW
		if (PSW & PSW_Z) {
			// update PC to the branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x24: // BNE/BNZ
		// check if zero flag is down on PSW
		if (!(PSW & PSW_Z)) {
			// update PC to the branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x28: // BC/BHS
		// check if carry flag is set on PSW
		if (PSW & PSW_C) {
			// update PC to branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x2C: // BNC/BLO
		// check if carry flag is down on PSW
		if (!(PSW & PSW_C)) {
			// update PC to branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x30: // BN
		// check if negative flag is set on PSW
		if (PSW & PSW_N) {
			// update PC to branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x34: // BGE
		// check if N and V are both set after subtraction
		if (((PSW & PSW_N) >> 2) && ((PSW & PSW_V) >> 4)) {
			// update PC to branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x38: // BLT
		// check if N != V (signs DO NOT match after subtraction)
		if (((PSW & PSW_N) >> 2) != ((PSW & PSW_V) >> 4)) {
			// update PC to branch address
			registerFile[R_PC] = instruction->operands[0];
		}
		return 0;

	case 0x3C: // BRA
		// update PC to the branch address
		registerFile[R_PC] = instruction->operands[0];
		return 0;

	default:
		return 1;
	}
}