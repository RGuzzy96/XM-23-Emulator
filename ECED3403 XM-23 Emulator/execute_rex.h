#ifndef EXECUTE_REX_H
#define EXECUTE_REX_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the register exchange instruction types
// returns 0/1/2 for execute return status code
int executeREX(Instruction* instruction);

#endif // !EXECUTE_REX_H