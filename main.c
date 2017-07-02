#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
#include "memory.h"

#define PROGFILE "./example/example.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *cpu;
	struct memory *mem;
	int ret;

	(void)argc;
	(void)argv;

	mem = memory_create("file-mem", PROGFILE);
	if(mem == NULL) {
		fprintf(stderr, "Cannot create memory segment\n");
		return -1;
	}

	ret = memory_map(mem, 0, 0, MEMSZ, MP_R | MP_W | MP_X);
	if(ret < 0) {
		fprintf(stderr, "Cannot map memory segment\n");
		return -1;
	}

	cpu = cpu_create("sparc", mem, NULL);
	if(cpu == NULL) {
		fprintf(stderr, "Cannot create cpu\n");
		return -1;
	}

	ret = cpu_boot(cpu, 0x0);
	if(ret < 0) {
		fprintf(stderr, "Cannot boot cpu\n");
		return -1;
	}

	while(1) {
		ret = cpu_fetch(cpu);
		if(ret < 0) {
			fprintf(stderr, "Cannot fetch instruction\n");
			goto exit;
		}

		ret = cpu_decode(cpu);
		if(ret < 0) {
			fprintf(stderr, "Cannot decode instruction\n");
			goto exit;
		}

		ret = cpu_exec(cpu);
		if(ret < 0) {
			fprintf(stderr, "Cannot execute instruction\n");
			goto exit;
		}
	}

exit:
	cpu_destroy(cpu);
	memory_unmap(mem, 0, MEMSZ);
	memory_destroy(mem);

	return 0;
}
