#ifndef EXECUTE_RIN_H
#define EXECUTE_RIN_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the register initialization instruction types
// returns 0/1/2 for execute return status code
int executeRIN(Instruction* instruction);

#endif // !EXECUTE_RIN_H

