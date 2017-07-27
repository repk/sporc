#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "../binaries/isa/xnorcc.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint32_t reg;
	uint8_t icc;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* XNORCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	reg = test_cpu_get_reg(c, 1);
	if(reg != 0xdeadbeef) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		goto close;
	}
	icc = test_cpu_get_cc_n(c);
	if(icc != 1) {
		fprintf(stderr, "Wrong icc n value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_z(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc z value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_v(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc v value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_c(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc c value after exec (%d)\n", icc);
		goto close;
	}

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* XNORCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	reg = test_cpu_get_reg(c, 1);
	if(reg != 0x00000000) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		goto close;
	}
	icc = test_cpu_get_cc_n(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc n value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_z(c);
	if(icc != 1) {
		fprintf(stderr, "Wrong icc z value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_v(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc v value after exec (%d)\n", icc);
		goto close;
	}
	icc = test_cpu_get_cc_c(c);
	if(icc != 0) {
		fprintf(stderr, "Wrong icc c value after exec (%d)\n", icc);
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	test_cpu_close(c);
exit:
	return ret;
}
