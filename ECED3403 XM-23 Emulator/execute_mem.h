#ifndef EXECUTE_MEM_H
#define EXECUTE_MEM_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the memory access instruction types
// returns 0/1/2 for execute return status code
int executeMEM(Instruction* instruction);

#endif // !EXECUTE_MEM_H

