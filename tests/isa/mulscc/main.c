#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "../binaries/isa/mulscc.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	uint64_t mul;
	int ret = -1;
	uint32_t r1, r2;
	size_t i;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* WRY */
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

	/* ORCC */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* 33 * mulscc */
	for(i = 0; i < 33; ++i) {
		ret = test_cpu_step(c);
		if(ret != 0)
			goto close;
	}

	/* RDY */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	r1 = test_cpu_get_reg(c, 1);
	r2 = test_cpu_get_reg(c, 2);
	mul = (((uint64_t)r2) << 32) | r1;
	if((unsigned long long)mul != 0xdeadbeefcacaULL) {
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
