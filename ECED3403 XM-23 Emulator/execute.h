#ifndef EXECUTE_H
#define EXECUTE_H

#include "decode.h"

// executes single instruction (just printing details, as of Lab 2)
int execute(Instruction* instruction);

// prepares error message based on integer error code returned by execute function
char* getErrMsg(int errCode);

#endif // !EXECUTE_H

