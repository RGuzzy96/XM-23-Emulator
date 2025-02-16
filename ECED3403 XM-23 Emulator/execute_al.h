#ifndef EXECUTE_AL_H
#define EXECUTE_AL_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the memory access instruction types
// returns 0/1/2 for execute return status code
int executeAL(Instruction* instruction);

#endif // !EXECUTE_AL_H

