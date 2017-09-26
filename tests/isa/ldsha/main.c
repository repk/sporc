#include <stdlib.h>
#include <stdio.h>
#include <endian.h>

#include <test-utils.h>
#include "cpu/cpu.h"
#include "dev/device.h"
#include "cpu/sparc/sparc.h"

#define PROGFILE "../binaries/isa/ldsha.bin"
#define KB 1024
#define MEMSZ (250 * KB)

struct __testasi_cfg {
	struct cpu *cpu;
};

struct __testasi_dev {
	struct dev dev;
	struct cpu *cpu;
};
#define to___testasi_dev(d) (container_of(d, struct __testasi_dev, dev))

static int __testasi_read16(struct dev *dev, addr_t addr, uint16_t *val)
{
	(void)dev;
	if(addr == 0xdeadbee0)
		*val = htobe16((uint16_t)(-12));
	else
		*val = 0;

	return 0;
}

static int __testasi_create(struct dev **dev, struct devcfg const *cfg)
{
	struct __testasi_cfg const *tcfg =
		(struct __testasi_cfg const*)cfg->cfg;
	struct __testasi_dev *d = NULL;
	int ret = -ENOMEM;

	d = malloc(sizeof(*d));
	if(d == NULL)
		goto err;

	d->cpu = tcfg->cpu;
	ret = scpu_register_mem(d->cpu, 0x12, &d->dev);
	if(ret != 0)
		goto err;

	*dev = &d->dev;
	return 0;
err:
	if(d)
		free(d);
	return ret;
}

static void __testasi_destroy(struct dev *dev)
{
	struct __testasi_dev *d = to___testasi_dev(dev);
	scpu_remove_mem(d->cpu, 0x12);
	free(d);
}

static struct devops const __testasiops = {
	.create = __testasi_create,
	.destroy = __testasi_destroy,
	.read16 = __testasi_read16,
};

static struct drv const __testasi = {
	.name = "__testasi",
	.ops = &__testasiops,
};
DRIVER_REGISTER(__testasi);

int main(int argc, char **argv)
{
	struct cpu *c = NULL;
	struct __testasi_cfg testdev_cfg;
	struct devcfg testdev = {
		.drvname = "__testasi",
		.name = "__testasi0",
		.cfg = &testdev_cfg,
	};
	struct dev *d = NULL;
	int ret = -1;
	uint32_t reg;

	c = test_cpu_open(argc, argv, PROGFILE, MEMSZ);
	if(c == NULL)
		goto exit;

	testdev_cfg.cpu = c;
	d = dev_create(&testdev);
	if(d == NULL)
		goto exit;

	/* SETHI */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* OR */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	/* Alt Load */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	reg = test_cpu_get_reg(c, 2);
	if(((int32_t)reg) != -12) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		ret = -1;
		goto close;
	}

	printf("[OK]\n");
	ret = 0;

close:
	if(d)
		dev_destroy(d);
	test_cpu_close(c);
exit:
	return ret;
}

