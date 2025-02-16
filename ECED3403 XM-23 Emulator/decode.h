#pragma once

#include <stdint.h>

// enum for possible types of instructions for XM-23
typedef enum {
	RIN,				   // register initialization
	MEM,				   // memory accessing
	AL,				       // arithmetic and logic
	REX,				   // register exchange
	SO,				       // single-operand
	TOC,				   // transfer of control
	SYS,				   // PSW and systems instructions
} InstructionType;

// struct for decoded instruction word
typedef struct {
	uint16_t opcode;
	char* mnemonic;
	InstructionType type;
	uint16_t operands[3];
	int8_t operandCount;
	int8_t rc;			   // register or constant
	int8_t wb;			   // word or byte
	int8_t inc;			   // increment
	int8_t dec;			   // decrement
	int8_t prpo;		   // pre or post increment/decrement
	uint8_t source;		   // the source register number or constant index (3 bits)
	uint8_t destination;   // the desination register number (3 bits)
} Instruction;

// function for decoding an instruction word into its opcode, operands, and flags
int decode(uint16_t nextInstructionWord, Instruction* instruction);

typedef struct {
	uint16_t opcode; 
	uint16_t opcodeMask; // mask to extract exact opcode if there is encoded data in its lower bits
	const char* mnemonic;
	uint8_t operandCount;
	InstructionType type;
} OpcodeInfo;

// table holding all the opcodes of the XM-23 ISA and their details required for unique decoding
// opcodes extended in this table to 16-bits to allow handling for the range of opcode sizes
static const OpcodeInfo opcodeTable[] = {
	{ 0x0000, 0xE000, "BL", 3, TOC },           // only 1 true operand for TOC instructions, but putting 3
	{ 0x2000, 0xFC00, "BEQ/BZ", 3, TOC },		// to store offset, current PC, and branch PC
	{ 0x2400, 0xFC00, "BNE/BNZ", 3, TOC },
	{ 0x2800, 0xFC00, "BC/BHS", 3, TOC },
	{ 0x2C00, 0xFC00, "BNC/BLO", 3, TOC },
	{ 0x3000, 0xFC00, "BN", 3, TOC },
	{ 0x3400, 0xFC00, "BGE", 3, TOC },
	{ 0x3800, 0xFC00, "BLT", 3, TOC },
	{ 0x3C00, 0xFC00, "BRA", 3, TOC },
	{ 0x4000, 0xFF00, "ADD", 2, AL },
	{ 0x4100, 0xFF00, "ADDC", 2, AL },
	{ 0x4200, 0xFF00, "SUB", 2, AL },
	{ 0x4300, 0xFF00, "SUBC", 2, AL },
	{ 0x4400, 0xFF00, "DADD", 2, AL },
	{ 0x4500, 0xFF00, "CMP", 2, AL },
	{ 0x4600, 0xFF00, "XOR", 2, AL },
	{ 0x4700, 0xFF00, "AND", 2, AL },
	{ 0x4800, 0xFF00, "OR", 2, AL },
	{ 0x4900, 0xFF00, "BIT", 2, AL },
	{ 0x4A00, 0xFF00, "BIC", 2, AL },
	{ 0x4B00, 0xFF00, "BIS", 2, AL },
	{ 0x4C00, 0xFF80, "MOV", 2, REX },
	{ 0x4C80, 0xFF80, "SWAP", 2, REX },
	{ 0x4D00, 0xFFB8, "SRA", 1, SO },
	{ 0x4D08, 0xFFB8, "RRC", 1, SO },
	{ 0x4D10, 0xFFB8, "COMP", 1, SO },
	{ 0x4D18, 0xFFB8, "SWPB", 1, SO },
	{ 0x4D20, 0xFFB8, "SXT", 1, SO },
	{ 0x5800, 0xFC00, "LD", 2, MEM },
	{ 0x5C00, 0xFC00, "ST", 2, MEM },
	{ 0x6000, 0xF800, "MOVL", 2, RIN },
	{ 0x6800, 0xF800, "MOVLZ", 2, RIN },
	{ 0x7000, 0xF800, "MOVLS", 2, RIN },
	{ 0x7800, 0xF800, "MOVH", 2, RIN },
	{ 0x8000, 0xC000, "LDR", 3, MEM },
	{ 0xC000, 0xC000, "STR", 3, MEM },
};

// struct for print info for a given instruction
typedef struct {
	const char* labels[3]; // array to store labels specific to different instruction types
	const char* formats[3]; // formatting for the values associated with the labels
} PrintInfo;