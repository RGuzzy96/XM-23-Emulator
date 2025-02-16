#ifndef EXECUTE_SO_H
#define EXECUTE_SO_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the single operand instruction types
// returns 0/1/2 for execute return status code
int executeSO(Instruction* instruction);

#endif // !EXECUTE_SO_H