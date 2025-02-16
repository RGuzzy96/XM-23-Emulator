#define _CRT_SECURE_NO_WARNINGS // to avoid errors on functions like sscanf

#include "cpu.h"
#include "fetch.h"
#include "decode.h"
#include "execute.h"
#include "memory.h"
#include "registers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t PC = 0x0000;

void initializePC(uint16_t address) {
	registerFile[R_PC] = address; // store PC in register 7
	printf("Program Counter initialized to 0x%04X\n", registerFile[R_PC]);
}

// function to handle user input for stepping through the program
static int handleUserCommand() {
	char input[10];

	// print instructions message for user
	printf("\n[ENTER} to continue | [S] to stop | [P <new PC>] to change PC | [R] to display registers | [W] to display PSW\n");
	printf(">");

	// get user input
	fgets(input, sizeof(input), stdin);

	// remove the newline character
	input[strcspn(input, "\n")] = '\0';

	// check if user pressed enter
	if (input[0] == '\0') {
		// return 1 to continue program
		printf("\n");
		return 1;
	}

	// check if user entered S or s
	if (input[0] == 'S' || input[0] == 's') {
		// return 0 to stop program
		printf("\n");
		return 0;
	}

	// check if user entered P and a new PC value
	if (input[0] == 'P' || input[0] == 'p') {
		unsigned int newPC; // using larger than uint16_t to handle overflow if user enters number too big
		// put remaining input string through sscanf to parse hex val
		if (sscanf(input + 1, "%x", &newPC) == 1) {
			// check if new PC is within memory range
			if (newPC > MEMORY_SIZE) {
				printf("Invalid PC value: Out of memory range, enter value between 0x0000-FFFF\n");
			}
			else {
				// set PC as new PC
				registerFile[R_PC] = newPC;
				printf("Program Counter updated to 0x%04x\n\n", registerFile[R_PC]);
				return 1;
			}
		}
		else {
			printf("Invalid PC value. Please enter in hex format\n");
		}
	}

	// check if user entered R or r
	if (input[0] == 'R' || input[0] == 'r') {
		displayRegisterFile();

		// ask for input again, in case user does not want to continue program yet
		return handleUserCommand();
	}

	// check if user entered W or w
	if (input[0] == 'W' || input[0] == 'w') {
		displayPSW();

		// ask for input again, in case user does not want to continue program yet
		return handleUserCommand();
	}

	// if no valid command, print message, flush input buffer, and call function again
	printf("Invalid command. Try again\n");
	while (getchar() != '\n');
	return handleUserCommand();
}

void cpuCycle() {

	printf("Starting cpu cycle...\n\n");

	// start loop
	while (1) {

		// fetch next instruction from memory
		uint16_t nextInstructionWord = fetch();

		// check if we have reached the end of our program instructions
		if (nextInstructionWord == 0x0000) {
			printf("End of program reached (0x0000 encountered).\n");
			break;
		}

		// decode instruction
		Instruction nextInstruction;
		 
		// attempt to decode and execute instruction (if two-register arithmetic or branching)
		if (decode(nextInstructionWord, &nextInstruction)) {
			int code = execute(&nextInstruction);
			if (code) {
				char* errMsg = getErrMsg(code);
				printf("Error executing instruction: %s\n", errMsg);
			}
		}
		else {
			// print hex word instruction for all other opcodes
			printf("Instruction: 0x%04x\n", nextInstructionWord);
		}
		
		// stop and wait for user to command next action
		if (!handleUserCommand()) {

		}
	}
}