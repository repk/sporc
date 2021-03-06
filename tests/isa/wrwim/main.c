#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include "cpu/cpu.h"

#define PROGFILE "../binaries/isa/wrwim.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint32_t reg, opc;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* call */
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

	/* or */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* wrwim */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Nop */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Nop */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Nop */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	opc = test_cpu_get_pc(c);

	/* restore */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	if(opc + 4 != test_cpu_get_pc(c)) {
		fprintf(stderr, "Unexpected trap\n");
		ret = -1;
		goto close;
	}

	/* restore */
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

	/* or */
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
