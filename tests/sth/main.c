#include <stdlib.h>
#include <stdio.h>
#include <endian.h>

#include <test-utils.h>
#include <cpu.h>

#define PROGFILE "./binaries/sth.bin"
#define KB 1024
#define MEMSZ (250 * KB)

int main(int argc, char **argv)
{
	struct cpu *c;
	int ret = -1;
	uint16_t mem;

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

	/* Store */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	mem = test_cpu_get_mem16(c, 0x10);
	if(mem != htobe16(0xbeef)) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", mem);
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	test_cpu_close(c);
exit:
	return ret;
}



