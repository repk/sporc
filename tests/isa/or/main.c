#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "../binaries/isa/or.bin"
#define KB 1024
#define MEMSZ (250 * KB)
#define NRINST 3

int main(int argc, char **argv)
{
	struct cpu *c;
	size_t i;
	int ret = -1;
	uint32_t reg;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	for(i = 0; i < NRINST; ++i) {
		ret = test_cpu_step(c);
		if(ret != 0)
			goto close;
	}

	reg = test_cpu_get_reg(c, 2);
	if(reg != 0xff0000ff) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	test_cpu_close(c);
exit:
	return ret;
}

