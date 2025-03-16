#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdio.h>
#include <stdint.h>

#define REGISTER_COUNT 8

// define special registers indices
#define R_LR 5		// link register, R5
#define R_PC 7		// program counter, R7

// define PSW bit positions
#define PSW_C (1 << 0)		// carry flag (bit 0)
#define PSW_Z (1 << 1)		// zero flag (bit 1)
#define PSW_N (1 << 2)		// negative flag (bit 2)
#define PSW_V (1 << 4)		// overflow flag (bit 3)

// define helper macros for setting and clearing PSW flags
#define SET_FLAG(flag) (PSW |= (flag))
#define CLEAR_FLAG(flag) (PSW &= ~(flag))
#define CHECK_FLAG(flag) (PSW &= (flag))

// define masks for PSW fields
#define PSW_PP_MASK   0xE000  // previous priority, (bits 15-13)
#define PSW_FLT_MASK  0x0100  // fault, bit 8
#define PSW_CP_MASK   0x00E0  // current priority, bits 7-5
#define PSW_V_MASK    0x0010  // overflow, bit 4
#define PSW_SLP_MASK  0x0008  // sleep, bit 3
#define PSW_N_MASK    0x0004  // negative, bit 2
#define PSW_Z_MASK    0x0002  // zero, bit 1
#define PSW_C_MASK    0x0001  // carry, bit 0

// struct defining our XM-23 register file
extern uint16_t registerFile[REGISTER_COUNT];

// define PSW register
extern uint16_t PSW; // PSW: 

// define available constants
static int8_t constants[REGISTER_COUNT] = { 0, 1, 2, 4, 8, 16, 32, -1 };

// function to initialize the register file - used at the start of program
void initializeRegisterFile();

// writes to register specified in word or byte mode, returns 0 for success, 2 for error
int writeToRegister(uint8_t registerIdentifier, uint16_t value, int isByteMode, int isMSB);

// reads from specified register, returning 1/0 for success/failure, and reading the value into the provided buffer
int readFromRegister(uint8_t registerIdentifier, uint16_t* buffer);

// prints the current register contents to the console
void displayRegisterFile();

// prints the PSW at the current moment in hex, binary, and broken into readable flags
void displayPSW();

// function to update status flags in PSW after an arithmetic/logic operation
void updateFlags(uint16_t result, uint16_t src, uint16_t dst, int isByteMode, int isSubtraction);

#endif // !REGISTERS_H
