/*

ECED3403 - Computer Architecture
Lab 2 - Instruction Decoding
Ryan Guzzwell (B00859689)
February 6, 2025

*/

#include "file_loader.h"
#include "file_decoder.h"
#include "memory.h"
#include "cpu.h"
#include "decode.h"
#include "registers.h"

int main(int argc, char* argv[]) {
	FILE* file = NULL;

	// check if a file was provided to the program
	if (argc > 1) {
		file = loadFile(argv[1]);
	}
	else {
		printf("No file provided!\n");
		return 1;
	}

	// check if we were able to open file
	if (file == NULL) {
		printf("Unable to open file\n");
		return 1;
	}

	// initialize simulated memory
	initializeMemory();

	// initialize XM-23 register file
	initializeRegisterFile();

	// decode the file and store raw instructions in memory
	decodeFile(file);

	// start fetch/decode/execute loop
	cpuCycle();

	// free memory when done
	cleanupMemory();

	// hold program until user decides to exit
	printf("Press any key to exit...\n");
	getchar();

	return 0;
}