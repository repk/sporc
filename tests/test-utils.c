#include <stdio.h>
#include <string.h>

#include <cpu.h>
#include <memory.h>
#include "test-utils.h"

#include "../cpu/sparc/sparc.h"

uint32_t test_cpu_get_reg(struct cpu *cpu, off_t ridx)
{
	return *scpu_get_reg(cpu, ridx);
}

int test_cpu_step(struct cpu *cpu)
{
	int ret;

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

exit:
	return ret;
}

struct cpu *test_cpu_open(int argc, char **argv, char const *memfile,
		size_t memsz)
{
	struct cpu *cpu = NULL;
	char file[FILENAME_MAX], *end;
	struct memory *mem;
	int ret;

	if(argc == 0) {
		fprintf(stderr, "Malformed prog args\n");
		goto err;
	}

	/* Get relative memfile path */
	end = strrchr(argv[0], '/');
	if(end) {
		snprintf(file, FILENAME_MAX - 1, "%.*s/%s",
				(int)(end - argv[0]), argv[0], memfile);
	} else {
		strncpy(file, memfile, FILENAME_MAX - 1);
	}
	file[FILENAME_MAX - 1] = '\0';

	mem = memory_create("file-mem", file);
	if(mem == NULL) {
		fprintf(stderr, "Cannot create memory segment\n");
		goto err;
	}

	ret = memory_map(mem, 0, 0, memsz, MP_R | MP_W | MP_X);
	if(ret < 0) {
		fprintf(stderr, "Cannot map memory segment\n");
		goto memexit;
	}

	cpu = cpu_create("sparc", mem, NULL);
	if(cpu == NULL) {
		fprintf(stderr, "Cannot create cpu\n");
		goto memunmap;
	}

	ret = cpu_boot(cpu, 0x0);
	if(ret < 0) {
		fprintf(stderr, "Cannot boot cpu\n");
		goto cpuexit;
	}

	return cpu;

cpuexit:
	cpu_destroy(cpu);
memunmap:
	memory_unmap(mem, 0, memsz);
memexit:
	memory_destroy(mem);
err:
	return NULL;
}

void test_cpu_close(struct cpu *cpu)
{
	memory_unmap(cpu->mem, 0, -1);
	memory_destroy(cpu->mem);
	cpu_destroy(cpu);
}
