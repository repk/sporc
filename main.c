#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

#define PROGFILE "./example/example.bin"

int main(int argc, char **argv)
{
	struct memory *mem;
	int ret;
	uint32_t rd;

	(void)argc;
	(void)argv;

	mem = memory_create("file-mem", PROGFILE);
	if(mem == NULL) {
		fprintf(stderr, "Cannot create memory segment\n");
		return -1;
	}

	ret = memory_map(mem, 0, 0, -1, MP_R | MP_W | MP_X);
	if(ret < 0) {
		fprintf(stderr, "Cannot map memory segment\n");
		return -1;
	}

	ret = memory_read32(mem, 0, &rd);
	if(ret < 0) {
		fprintf(stderr, "Cannot read memory segment\n");
		return -1;
	}

	printf("Memory mapped with first val 0x%08x\n", be32toh(rd));

	memory_unmap(mem, 0, -1);
	memory_destroy(mem);

	return 0;
}
