#include "file_loader.h"

FILE* loadFile(const char *filename) {
	FILE* file = fopen(filename, "r");
	return file;
};