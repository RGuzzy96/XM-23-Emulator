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
#include <time.h>
#include <signal.h>

uint32_t cpuClock = 0;
uint16_t breakPoint = 0;
int executionSpeedMode = 1; // 0 - slow, 1 - normal, 2 - fast

int braCount = 0; // counter to detect possible end of program on an infinite loop
int braStopIgnored = 0;

volatile sig_atomic_t ctrl_c_fnd; // control c flag

// time delay settings
#define SLOW_DELAY		500		// slow mode (0.5 seconds delay per cycle)
#define NORMAL_DELAY	100		// normal (0.1s)
#define FAST_DELAY		10		// fast (0.01s)	

// function to hamdle SIGINT (^C)
void sigint_hdlr(int signum) {
	ctrl_c_fnd = 1;
	signal(SIGINT, (_crt_signal_t)sigint_hdlr);
}

void initializeCtrlCHandler() {
	ctrl_c_fnd = 0;
	signal(SIGINT, (_crt_signal_t)sigint_hdlr); // bind handler to SIGINT
}

void initializePC(uint16_t address) {
	registerFile[R_PC] = address; // store PC in register 7
	printf("Program Counter initialized to 0x%04X\n", registerFile[R_PC]);
}

// function to delay between program step executions
static void delayExecution() {
	clock_t start_time = clock();
	clock_t delay_time;

	// get delay for selected speed mode
	switch (executionSpeedMode) {
		case 0: delay_time = SLOW_DELAY; break;
		case 1: delay_time = NORMAL_DELAY; break;
		case 2: delay_time = FAST_DELAY; break;
		default: delay_time = NORMAL_DELAY; break;
	}

	// loop until delay is up
	while ((clock() - start_time) < delay_time);
}

// function to ask and recieve potential break point from user
static void getBreakPoint() {
	char input[10];

	// ask for breakpoint
	printf("\nAdd a breakpoint? Enter [Y <PC value>] | [N]\n");
	printf(">");

	// read breakpoint response
	fgets(input, sizeof(input), stdin);

	// check if breakpoint added
	if (input[0] == 'Y' || input[0] == 'y') {
		unsigned int newBreakPoint;
		// put remaining input string through sscanf to parse hex val
		if (sscanf(input + 1, "%x", &newBreakPoint) == 1) {
			// check if new PC is within memory range
			if (newBreakPoint > MEMORY_SIZE) {
				printf("Invalid breakpoint value: Out of memory range, enter value between 0x0000-FFFF\n");
				// call recursively
				getBreakPoint();
			}
			else {
				// set breakpoint as new breakpoint
				breakPoint = newBreakPoint;
				printf("Breakpoint updated to 0x%04x\n\n", breakPoint);
			}
		}
		else {
			printf("Invalid breakpoint value. Please enter in hex format\n");
			// call recursively
			getBreakPoint();
		}
	}
	else {
		breakPoint = 0;
	}
}

// function to ask user for execution speed
static void getExecutionSpeed() {
	char input[10];

	printf("Select execution speed: [0] Slow | [1] Normal | [2] Fast\n");
	printf(">");

	fgets(input, sizeof(input), stdin);
	int speedChoice = atoi(input);

	if (speedChoice >= 0 && speedChoice <= 2) {
		executionSpeedMode = speedChoice;
		printf("Execution speed set to %s mode.\n\n",
			(executionSpeedMode == 0) ? "Slow" :
			(executionSpeedMode == 1) ? "Normal" : "Fast");
	}
	else {
		printf("Invalid choice. Defaulting to Normal speed.\n");
		executionSpeedMode = 1;
	}
}

// function to handle user input for stepping through the program
static int handleUserCommand() {
	char input[10];

	// print instructions message for user
	printf("\n[ENTER} to continue | [S] to stop | [P <new PC>] to change PC | [R] to display registers | [W] to display PSW | [D <addr> <len>] to dump memory | [B] to add breakpoint and continue | [V] to change speed and continue <\n");
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

	// check if user entered D or d
	if (input[0] == 'D' || input[0] == 'd') {
		unsigned int startAddr, length;
		// scan in values
		if (sscanf(input + 1, "%x %d", &startAddr, &length) == 2) {
			// print specified memory section
			printMemorySection(startAddr, length);

			// ask for input again
			while (getchar() != '\n'); // flush input buffer
			return handleUserCommand();
		}
		else {
			printf("Invalid input. Usage: D <start_address (hex)> <length (decimal)>\n");
		}
	}

	// check if user entered B or b
	if (input[0] == 'B' || input[0] == 'b') {
		getBreakPoint();
		return 1;
	}

	// check if user entered V or v
	if (input[0] == 'V' || input[0] == 'v') {
		getExecutionSpeed();
		return 1;
	}

	// if no valid command, print message, flush input buffer, and call function again
	printf("Invalid command. Try again\n");
	while (getchar() != '\n');
	return handleUserCommand();
}

// function takes and potentially modifies runMode and breakPoint control variables based on user input
static void getRunModeAndBreak(int *runMode) {
	// give user instructions
	printf("Selected program run mode: [S] step | [C] continuous");
	printf(">");

	char input[10];

	// read in input
	fgets(input, sizeof(input), stdin);

	// check if user entered C or c
	if (input[0] == 'C' || input[0] == 'c') {
		*runMode = 1;
		// ask for optional breakpoint
		getBreakPoint();

		// ask for desired speed mode
		getExecutionSpeed();
	}
}

void cpuCycle() {

	printf("Starting cpu cycle...\n\n");

	int runMode = 0; // 0 for step, 1 for continuous

	getRunModeAndBreak(&runMode);

	initializeCtrlCHandler();

	// start loop
	while (1) {
		
		// check if we may be at end of program loop due to repeated BRA
		if (braCount >= 5 && !braStopIgnored) {
			printf("\nRepeated BRA instructions - the program may be complete and in a loop.\n");
			// stop and wait for user to command next action
			if (!handleUserCommand()) {
				break;
			}
			braStopIgnored = 1;
		}

		// increment clock for fetch
		cpuClock += 1;

		// fetch next instruction from memory
		uint16_t nextInstructionWord = fetch();

		// check if we have reached the end of our program instructions
		if (nextInstructionWord == 0x0000) {
			printf("End of program reached (0x0000 encountered).\n");
			break;
		}

		// increment clock for decode
		cpuClock += 1;

		// decode instruction
		Instruction nextInstruction;
		 
		// attempt to decode and execute instruction (if two-register arithmetic or branching)
		if (decode(nextInstructionWord, &nextInstruction)) {
			int executionCycles = 1;

			// if memory access is involved, bump up execution cycles taken
			if (nextInstruction.type == MEM) {
				executionCycles += 3;
			}

			// check if BRA
			if (nextInstruction.mnemonic == "BRA") {
				braCount++;
			}
			else {
				// reset count if not BRA
				braCount = 0;
			}

			int code = execute(&nextInstruction);
			if (code) {
				char* errMsg = getErrMsg(code);
				printf("Error executing instruction: %s\n", errMsg);
			}

			// increment clock for execution
			cpuClock += executionCycles;
		}
		else {
			// print hex word instruction for all other opcodes
			printf("Instruction: 0x%04x\n", nextInstructionWord);
		}

		// print CPU clock
		printf("\nCPU Clock: %d\n", cpuClock);

		// delay next execution
		delayExecution();

		// check if we are in step run mode or have encountered a break point
		if (!runMode || registerFile[R_PC] == breakPoint || ctrl_c_fnd) {
			if (registerFile[R_PC] == breakPoint) {
				// let user know breakpoint encountered
				printf("\nBreakpoint encountered!\n");
			}
			else if (ctrl_c_fnd) {
				// let user know ctrl c worked
				printf("\n^C\n");
			}
			
			// stop and wait for user to command next action
			if (!handleUserCommand()) {
				break;
			}

			if (ctrl_c_fnd) {
				ctrl_c_fnd = 0;
			}
		}
	}
}