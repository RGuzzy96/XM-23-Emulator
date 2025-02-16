#ifndef EXECUTE_TOC_H
#define EXECUTE_TOC_H

#include "decode.h"
#include <stdint.h>

// executes instruction belonging to the transfer of control instruction types
// returns 0/1/2 for execute return status code
int executeTOC(Instruction* instruction);

#endif // !EXECUTE_TOC_H

