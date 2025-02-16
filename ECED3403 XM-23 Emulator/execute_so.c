#include "execute_so.h"

int executeSO(Instruction* instruction) {
	uint16_t dstValue, result;
	int16_t srcValue;
	int isByteMode = instruction->wb;

	switch (instruction->type) {
		case 0x4D00: // SRA
		case 0x4D08: // RRC
		case 0x4D10: // COMP
		case 0x4D18: // SWPB
		case 0x4D20: // SXT
		default:
			return 1;
	}
}