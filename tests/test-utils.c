#include <stdio.h>
#include <string.h>

#include "types.h"

#include "cpu/cpu.h"
#include "dev/device.h"
#include "dev/cfg/ramctl.h"
#include "dev/cfg/filemem.h"
#include "dev/cfg/sparc-nommu.h"

#include "test-utils.h"

#include "../cpu/sparc/sparc.h"

uint8_t test_cpu_get_cc_n(struct cpu *cpu)
{
	return scpu_get_cc_n(cpu);
}

uint8_t test_cpu_get_cc_z(struct cpu *cpu)
{
	return scpu_get_cc_z(cpu);
}

uint8_t test_cpu_get_cc_v(struct cpu *cpu)
{
	return scpu_get_cc_v(cpu);
}

uint8_t test_cpu_get_cc_c(struct cpu *cpu)
{
	return scpu_get_cc_c(cpu);
}

uint32_t test_cpu_get_reg(struct cpu *cpu, off_t ridx)
{
	return scpu_get_reg(cpu, ridx);
}

uint32_t test_cpu_get_y(struct cpu *cpu)
{
	uint32_t y;
	scpu_get_asr(cpu, 0, &y);
	return y;
}

uint32_t test_cpu_get_pc(struct cpu *cpu)
{
	return scpu_get_pc(cpu);
}

uint32_t test_cpu_get_mem32(struct cpu *cpu, addr_t addr)
{
	uint32_t ret = 0;
	dev_read32(scpu_get_dmem(cpu), addr, &ret);
	return ret;
}

uint16_t test_cpu_get_mem16(struct cpu *cpu, addr_t addr)
{
	uint16_t ret = 0;
	dev_read16(scpu_get_dmem(cpu), addr, &ret);
	return ret;
}

uint8_t test_cpu_get_mem8(struct cpu *cpu, addr_t addr)
{
	uint8_t ret = 0;
	dev_read8(scpu_get_dmem(cpu), addr, &ret);
	return ret;
}

int test_cpu_step(struct cpu *cpu)
{
	int ret;

	ret = cpu_fetch(cpu);
	if(ret < 0) {
		fprintf(stderr, "Cannot fetch instruction\n");
		goto exit;
	}

	ret = cpu_decode(cpu);
	if(ret < 0) {
		fprintf(stderr, "Cannot decode instruction\n");
		goto exit;
	}

	ret = cpu_exec(cpu);
	if(ret < 0) {
		fprintf(stderr, "Cannot execute instruction\n");
		goto exit;
	}

exit:
	return ret;
}

/* Cpu description */
static struct cpucfg const cpucfg = {
	.cpu = "sparc",
	.name = "cpu0",
};

/* Platform devices configuration */
static struct devcfg devcfg[] = {
	{
		.drvname = "file-mem",
		.name = "progmap",
	},
	{
		.drvname = "ramctl",
		.name = "ram0",
		.cfg = DEVCFG(ramctl_cfg) {
			.devlst = (struct rammap[]){
				{
					.devname = "progmap",
					.addr = 0x0,
					.perm = MP_R | MP_W | MP_X,
					.sz = -1,
				},
				{}, /* Sentinel */
			},
		},
	},
	{
		.drvname = "sparc-nommu",
		.name = "mmu0",
		.cfg = DEVCFG(sparc_nommu_cfg) {
			.dmem = "ram0",
			.imem = "ram0",
			.cpu = "cpu0",
		}
	},
};

struct cpu *test_cpu_open(int argc, char **argv, char const *memfile,
		size_t memsz)
{
	struct filemem_cfg fc = {
		.off = 0,
		.sz = memsz,
	};
	struct cpu *cpu = NULL;
	struct dev *d;
	char file[FILENAME_MAX], *end;
	size_t i;
	int ret;

	if(argc == 0) {
		fprintf(stderr, "Malformed prog args\n");
		goto err;
	}

	/* Get relative memfile path */
	end = strrchr(argv[0], '/');
	if(end) {
		snprintf(file, FILENAME_MAX - 1, "%.*s/%s",
				(int)(end - argv[0]), argv[0], memfile);
	} else {
		strncpy(file, memfile, FILENAME_MAX - 1);
	}
	file[FILENAME_MAX - 1] = '\0';
	fc.path = file;
	devcfg[0].cfg = &fc;

	/* Create Cpu */
	cpu = cpu_create(&cpucfg);
	if(cpu == NULL) {
		fprintf(stderr, "Cannot create cpu\n");
		goto err;
	}

	/* Create devices */
	for(i = 0; i < ARRAY_SIZE(devcfg); ++i) {
		if(dev_create(&devcfg[i]) == NULL) {
			fprintf(stderr, "Cannot create dev %s\n",
					devcfg[i].name);
			goto cpuexit;
		}
	}

	ret = cpu_boot(cpu, 0x0);
	if(ret < 0) {
		fprintf(stderr, "Cannot boot cpu\n");
		goto devexit;
	}

	return cpu;

devexit:
	for(i = ARRAY_SIZE(devcfg); i > 0; --i)
		if((d = dev_get(devcfg[i - 1].name)) != NULL)
			dev_destroy(d);
cpuexit:
	cpu_destroy(cpu);
err:
	return NULL;
}

void test_cpu_close(struct cpu *cpu)
{
	struct dev *d;
	size_t i;

	for(i = ARRAY_SIZE(devcfg); i > 0; --i)
		if((d = dev_get(devcfg[i - 1].name)) != NULL)
			dev_destroy(d);

	cpu_destroy(cpu);
}
