#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "../binaries/isa/smulcc.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	uint64_t mul;
	int ret = -1;
	uint32_t r1, r2;

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

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* SMULCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* BPOS,A */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* RDY */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	r1 = test_cpu_get_reg(c, 1);
	r2 = test_cpu_get_reg(c, 2);
	mul = (((uint64_t)r2) << 32) | r1;
	if((unsigned long long)mul != 0xfe8e7fb6b6960722ULL) {
		fprintf(stderr, "Wrong register value after exec 0x%llx\n",
				(unsigned long long)mul);
		ret = -1;
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

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* SMULCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* BNE,A */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* RDY */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	r1 = test_cpu_get_reg(c, 1);
	r2 = test_cpu_get_reg(c, 2);
	mul = (((uint64_t)r2) << 32) | r1;
	if((unsigned long long)mul != 0xfa749a8100000000ULL) {
		fprintf(stderr, "Wrong register value after exec 0x%llx\n",
				(unsigned long long)mul);
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
