#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include "cpu/cpu.h"

#define PROGFILE "../binaries/isa/stbar.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint8_t mem;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Store */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Store */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Stbar */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	mem = test_cpu_get_mem8(c, 0x12);
	if(mem != 0xc) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", mem);
		ret = -1;
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	test_cpu_close(c);
exit:
	return ret;
}
