#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpu/cpu.h"
#include "memory.h"
#include "utils.h"

#define PROGFILE "./example/example.bin"
#define KB 1024
#define MEMSZ (250 * KB)


/* Sparc cpu configuration */
static struct cpucfg cpucfg = {
	.cpu = "sparc",
	.name = "cpu0",
};

int get_file_path(int argc, char **argv, char *file,
		char *path, size_t sz)
{
	char *ptr;

	if(argc == 0)
		return -1;

	ptr = strrchr(argv[0], '/');
	if(ptr) {
		snprintf(path, sz - 1, "%.*s/%s", (int)(ptr - argv[0]),
				argv[0], file);
	} else {
		strncpy(path, file, sz - 1);
	}
	path[sz - 1] = '\0';

	return 0;
}

int main(int argc, char **argv)
{
	struct cpu *cpu;
	int ret;
	char f[FILENAME_MAX];

	/* Configure file path */
	ret = get_file_path(argc, argv, PROGFILE, f, ARRAY_SIZE(f));
	if(ret != 0) {
		fprintf(stderr, "Cannot get file path\n");
		return -1;
	}

	cpucfg.mem = memory_create("file-mem", f);
	if(cpucfg.mem == NULL) {
		fprintf(stderr, "Cannot create memory segment\n");
		return -1;
	}

	ret = memory_map(cpucfg.mem, 0, 0, MEMSZ, MP_R | MP_W | MP_X);
	if(ret < 0) {
		fprintf(stderr, "Cannot map memory segment\n");
		return -1;
	}

	/* Create Cpu */
	cpu = cpu_create(&cpucfg);
	if(cpu == NULL) {
		fprintf(stderr, "Cannot create cpu\n");
		return -1;
	}

	ret = cpu_boot(cpu, 0x0);
	if(ret < 0) {
		fprintf(stderr, "Cannot boot cpu\n");
		goto exit;
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
	memory_unmap(cpucfg.mem, 0, MEMSZ);
	memory_destroy(cpucfg.mem);

	return 0;
}
