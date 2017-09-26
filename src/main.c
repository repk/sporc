#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "cpu/cpu.h"
#include "dev/device.h"
#include "dev/cfg/ramctl.h"
#include "dev/cfg/filemem.h"
#include "dev/cfg/sparc-nommu.h"

#define PROGFILE "./example/example.bin"
#define KB 1024
#define MEMSZ (250 * KB)


/* Sparc cpu configuration */
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

int get_file_path(int argc, char **argv, char *file,
		char *path, size_t sz)
{
	char *ptr;

	if(argc == 0)
		return -1;

	ptr = strrchr(argv[0], '/');
	if(ptr) {
		snprintf(path, sz - 1, "%.*s/%s", (int)(ptr - argv[0]),
				argv[0], file);
	} else {
		strncpy(path, file, sz - 1);
	}
	path[sz - 1] = '\0';

	return 0;
}

int main(int argc, char **argv)
{
	struct filemem_cfg fc = {
		.off = 0,
		.sz = MEMSZ,
	};
	struct cpu *cpu;
	struct dev *d;
	size_t i;
	int ret;
	char f[FILENAME_MAX];

	/* Configure file path */
	ret = get_file_path(argc, argv, PROGFILE, f, ARRAY_SIZE(f));
	if(ret != 0) {
		fprintf(stderr, "Cannot get file path\n");
		return -1;
	}
	fc.path = f;
	devcfg[0].cfg = &fc;

	/* Create Cpu */
	cpu = cpu_create(&cpucfg);
	if(cpu == NULL) {
		fprintf(stderr, "Cannot create cpu\n");
		return -1;
	}

	/* Create devices */
	for(i = 0; i < ARRAY_SIZE(devcfg); ++i) {
		if(dev_create(&devcfg[i]) == NULL) {
			fprintf(stderr, "Cannot create dev %s\n",
					devcfg[i].name);
		}
	}

	ret = cpu_boot(cpu, 0x0);
	if(ret < 0) {
		fprintf(stderr, "Cannot boot cpu\n");
		goto exit;
	}

	while(1) {
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
	}

exit:
	for(i = ARRAY_SIZE(devcfg); i > 0; --i)
		if((d = dev_get(devcfg[i - 1].name)) != NULL)
			dev_destroy(d);

	cpu_destroy(cpu);

	return 0;
}
