#include <stdlib.h>
#include <stdio.h>

#include <test-utils.h>
#include "cpu/cpu.h"
#include "dev/device.h"
#include "cpu/sparc/sparc.h"

#define PROGFILE "../binaries/isa/ldstuba.bin"
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

static uint8_t __testasi_mval;

static int __testasi_read8(struct dev *dev, addr_t addr, uint8_t *val)
{
	(void)dev;
	if(addr == 0xdeadbeef)
		*val = (uint8_t)(-12);
	else
		*val = 0;

	return 0;
}

static int __testasi_write8(struct dev *dev, addr_t addr, uint8_t val)
{
	(void)dev;
	if(addr == 0xdeadbeef)
		__testasi_mval = val;

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
	.read8 = __testasi_read8,
	.write8 = __testasi_write8,
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

	/* Alt ldstub */
	ret = test_cpu_step(c);
	if(ret != 0)
		goto close;

	reg = test_cpu_get_reg(c, 2);
	if(((uint8_t)reg) != 0xf4) {
		fprintf(stderr, "Wrong register value after exec 0x%x\n", reg);
		ret = -1;
		goto close;
	}

	if(__testasi_mval != 0xff) {
		fprintf(stderr, "Wrong memory value after exec 0x%x\n",
				__testasi_mval);
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

