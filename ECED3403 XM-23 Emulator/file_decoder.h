#ifndef FILE_DECODER_H
#define FILE_DECODER_H

#include <stdio.h>
#include <stdlib.h>

// S-record types expected from XM-23 assembler
enum RecordTypes { S0, S1, S9 };

// decodes file and stores raw instructions in memory
void decodeFile(FILE* file);

#endif // !FILE_DECODER_H
