#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "../binaries/isa/rdpsr.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint32_t reg;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	/* Restore */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Subcc */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* rdpsr */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Test Z and Supervisor flag and CWP == 1 */
	reg = test_cpu_get_reg(c, 1);
	if(reg != ((1 << 22) | (1 << 7) | 1)) {
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
