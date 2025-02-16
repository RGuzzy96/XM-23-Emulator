#include "file_decoder.h"
#include "memory.h"
#include "cpu.h"

#define MAX_RECORD_LENGTH 81 // sources say max length ranges from 64 - 80 characters, going with highest 
							 // (sources: https://www.systutorials.com/docs/linux/man/5-srec/, https://srecord.sourceforge.net/reference-1.65.pdf)

static int toHex(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0'; // convert ASCII character to decimal number
	}
	else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10; // convert ASCII 'A-F' to decimal 10-15
	}
	else {
		printf("Invalid hex character: %c", c);
		return -1;
	}
}

static char ASCIIToHexByte(char* array, int offset, int counter) {
	char msb = array[offset + 2 * counter];
	char lsb = array[offset + 2 * counter + 1];
	unsigned char hex = (toHex(msb) << 4) | toHex(lsb); // convert to single hex byte
	return hex;
}

static int getRecordLength(char* record) {
	return toHex(record[2]) * 16 + toHex(record[3]);
}

static int getRecordCheckSum(char* record, int length) {
	int msb = toHex(record[(length + 1) * 2]);
	int lsb = toHex(record[(length + 1) * 2 + 1]);
	return msb * 16 + lsb;
}

static int getRecordAddrField(char* record) {
	return (toHex(record[4]) << 12) |
		(toHex(record[5]) << 8) |
		(toHex(record[6]) << 4) |
		toHex(record[7]);
}

static int getAddrAsIntSum(int address) {
	int addrMsb = (address >> 8) & 0xFF; // shift MSB and mask so we are only considering that byte
	int addrLsb = address & 0xFF; // directly mask the LSB so we are only considering that byte now
	return addrMsb + addrLsb;
}

static int validateChecksum(int checksum, int sum) {
	int sumLsb = sum & 0xFF; // take the lsb
	int calculatedChecksum = (~sumLsb) & 0xFF;

	if (calculatedChecksum == checksum) {
		return 1;
	}
	else {
		return 0;
	}
}

static void decodeType0(char *record) {
	printf("--------- Decoding S0 record ---------\n\n");

	int recordLength = getRecordLength(record);
	int checkSum = getRecordCheckSum(record, recordLength);
	int rollingSum = recordLength;

	char* filename = (char*)malloc((recordLength - 3) + 1);

	if (filename == NULL) {
		printf("Memory allocation failed for filename");
		return;
	}

	for (int i = 0; i < (recordLength - 3); i++) {
		unsigned char hex = ASCIIToHexByte(record, 8, i);
		filename[i] = hex; // adding 8 to start after S-rec type, length, and address field
		rollingSum += hex;
	}

	filename[recordLength - 3] = '\0'; // null terminate the string

	if (validateChecksum(checkSum, rollingSum)) {
		printf("Filename: %s\n\n", filename);
	}
	else {
		printf("S0 record has invalid checksum! Record ignored\n\n");
	}
}

static void decodeType1(char *record) {
	printf("--------- Decoding S1 record ---------\n\n");

	int recordLength = getRecordLength(record);

	int checkSum = getRecordCheckSum(record, recordLength);
	int rollingSum = recordLength;

	unsigned char* data = (unsigned char*)malloc(recordLength - 3);

	int address = getRecordAddrField(record);
	rollingSum += getAddrAsIntSum(address);

	for (int i = 0; i < (recordLength - 3); i++) {
		
		// get ASCII element of record as single hex byte
		unsigned char hex = ASCIIToHexByte(record, 8, i);
		rollingSum += hex;
		data[i] = hex;
	}

	if (validateChecksum(checkSum, rollingSum)) {
		printf("Starting address: 0x%04X\n", address);
		writeArrayToMemory(address, data, recordLength - 3);
		printf("\nMemory written:\n\n");
		printMemorySection(address, recordLength - 3);
		printf("\n");
	}
	else {
		printf("S1 record has invalid checksum! Data not written to memory\n\n");
	}
}

static void decodeType9(char *record) {
	printf("--------- Decoding S9 record ---------\n\n");

	int recordLength = getRecordLength(record);
	int checkSum = getRecordCheckSum(record, recordLength);
	int rollingSum = recordLength;

	int address = getRecordAddrField(record);
	rollingSum += getAddrAsIntSum(address);

	for (int i = 0; i < (recordLength - 3); i++) {
		unsigned char hex = ASCIIToHexByte(record, 8, i);
		rollingSum += hex;
	}

	if (validateChecksum(checkSum, rollingSum)) {
		printf("Starting address: 0x%04X\n", address);
		printf("\n");

		// initialize program counter to starting address
		initializePC(address);
	}
	else {
		printf("S9 record has invalid checksum! Data not written to memory\n\n");
	}
}

static void processRecord(char *record) {

	// verify it begins with S
	if (record[0] != 'S') {
		printf("Invalid S-Record, ignoring line\n");
		return;
	}

	char type = record[1];

	switch (type) {
		case '0':
			decodeType0(record);
			break;
		case '1':
			decodeType1(record);
			break;
		case '9':
			decodeType9(record);
			break;
	}
}

void decodeFile(FILE* file) {
	char* record[MAX_RECORD_LENGTH];

	printf("Decoding file...\n\n");
	
	while (fgets(record, sizeof(record), file)) {
		processRecord(record);
	}
};