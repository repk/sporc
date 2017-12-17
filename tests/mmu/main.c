#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include "cpu/cpu.h"

#define PROGFILE "../binaries/mmu/mmu.bin"
#define KB 1024
#define MEMSZ (250 * KB)
#define NRINST 28

int main(int argc, char **argv)
{
	struct cpu *c;
	size_t i;
	int ret = -1;
	uint32_t reg;

	c = test_mmucpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	for(i = 0; i < NRINST; ++i) {
		ret = test_cpu_step(c);
		if(ret != 0)
			goto close;
	}

	reg = test_cpu_get_reg(c, 1);
	if(reg != 0xdeadbeef) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		ret = -1;
		goto close;
	}

	reg = test_cpu_get_reg(c, 2);
	if(reg != 0xdeadbeef) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		ret = -1;
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	test_mmucpu_close(c);
exit:
	return ret;
}


