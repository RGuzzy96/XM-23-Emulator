#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#define BUS_READ 0
#define BUS_WRITE 1

// verifies valid memory access and reads into or writes provided value based on mode
int bus(uint16_t address, uint8_t* value, int mode);

#endif // !BUS_H

