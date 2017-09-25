#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include "cpu/cpu.h"

#define PROGFILE "../binaries/isa/tvc.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint32_t reg, spc;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* Call */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Nop */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* RDPSR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* WRPSR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* NOP */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* NOP */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* NOP */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Sethi */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* ADDCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	spc = test_cpu_get_pc(c);

	/* TVC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	if(spc + 4 != test_cpu_get_pc(c)) {
		fprintf(stderr, "Wrong should not trap here\n");
		ret = -1;
		goto close;
	}

	/* SUBCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* TVC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Call */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Nop */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* sethi */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	reg = test_cpu_get_reg(c, 1);
	if(reg != 0xdeadbeef) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
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
