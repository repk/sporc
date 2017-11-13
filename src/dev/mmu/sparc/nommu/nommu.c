#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "dev/device.h"
#include "dev/cfg/mmu/sparc/nommu.h"
#include "cpu/cpu.h"
#include "cpu/sparc/sparc.h"

struct snommu;
/* sparc MMU device (data/instruction) configuration */
struct sparc_nommu_dev_cfg {
	struct snommu *mmu;
	char const *mem;
};

/* sparc MMU virtual device (data/instruction) */
struct snommu_dev {
	struct dev dev;
	struct dev *mem; /* Memory controller device */
	struct snommu *mmu;
};
#define to_snommu_dev(d) (container_of(d, struct snommu_dev, dev))

/* Global MMU device */
struct snommu {
	struct dev dev; /* Sparc nommu global device */
	struct snommu_dev data; /* Sparc nommu data specific virtual device */
	struct snommu_dev isn; /*Sparc nommu isn specific virtual device */
	struct cpu *cpu; /* Sparc CPU */
};
#define to_snommu(d) (container_of(d, struct snommu, dev))

/**
 * Fetch a 8 bit value from memory
 */
static int snommu_read8(struct dev *dev, addr_t addr, uint8_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->read8)
		return -ENOSYS;
	return mem->drv->phyops->read8(mem, addr, val);
}

/**
 * Fetch a 16 bit value from memory
 */
static int snommu_read16(struct dev *dev, addr_t addr, uint16_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->read16)
		return -ENOSYS;
	return mem->drv->phyops->read16(mem, addr, val);
}

/**
 * Fetch a 32 bit value from memory
 */
static int snommu_read32(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->read32)
		return -ENOSYS;
	return mem->drv->phyops->read32(mem, addr, val);
}

/**
 * Write a 8 bit value into memory
 */
static int snommu_write8(struct dev *dev, addr_t addr, uint8_t val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->write8)
		return -ENOSYS;
	return mem->drv->phyops->write8(mem, addr, val);
}

/**
 * Write a 16 bit value into memory
 */
static int snommu_write16(struct dev *dev, addr_t addr, uint16_t val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->write16)
		return -ENOSYS;
	return mem->drv->phyops->write16(mem, addr, val);
}

/**
 * Write a 32 bit value into memory
 */
static int snommu_write32(struct dev *dev, addr_t addr, uint32_t val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->write32)
		return -ENOSYS;
	return mem->drv->phyops->write32(mem, addr, val);
}

/**
 * Fetch a 8 bit value from memory with execute permission
 */
static int snommu_fetch_isn8(struct dev *dev, addr_t addr, uint8_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->fetch_isn8)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn8(mem, addr, val);
}

/**
 * Fetch a 16 bit value from memory with execute permission
 */
static int snommu_fetch_isn16(struct dev *dev, addr_t addr, uint16_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->fetch_isn16)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn16(mem, addr, val);
}

/**
 * Fetch a 32 bit value from memory with execute permission
 */
static int snommu_fetch_isn32(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct dev *mem = to_snommu_dev(dev)->mem;

	if(!mem->drv->phyops->fetch_isn32)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn32(mem, addr, val);
}

/**
 * Create a new sparc fixed mmu instruction access virtual device
 *
 * @param dev: Device to initialize
 * @param cfg: New device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int snommu_isn_create(struct dev **dev, struct devcfg const *cfg)
{
	struct sparc_nommu_dev_cfg const *scfg =
		(struct sparc_nommu_dev_cfg const*)cfg->cfg;
	struct snommu_dev *mdev = &scfg->mmu->isn;
	int ret = -ENODEV;

	*dev = NULL;

	mdev->mem = dev_get(scfg->mem);
	if(mdev->mem == NULL)
		goto err;

	mdev->mmu = scfg->mmu;

	ret = scpu_register_mem(mdev->mmu->cpu, SPARC_AS_UISN, &mdev->dev);
	if(ret != 0)
		goto err;

	ret = scpu_register_mem(mdev->mmu->cpu, SPARC_AS_SISN, &mdev->dev);
	if(ret != 0)
		goto uisnrm;

	*dev = &mdev->dev;
	return 0;

uisnrm:
	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_UISN);
err:
	return ret;
}

/**
 * Destroy a sparc fixed mmu instruction access virtual device
 *
 * @param dev: Device to be removed
 *
 * @return: 0 on success, negative error otherwise
 */
static void snommu_isn_destroy(struct dev *dev)
{
	struct snommu_dev *mdev = to_snommu_dev(dev);

	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_UISN);
	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_SISN);
}

/**
 * Create a new sparc fixed mmu data access virtual device
 *
 * @param dev: New device to create
 * @param cfg: New device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int snommu_data_create(struct dev **dev, struct devcfg const *cfg)
{
	struct sparc_nommu_dev_cfg const *scfg =
		(struct sparc_nommu_dev_cfg const*)cfg->cfg;
	struct snommu_dev *mdev = &scfg->mmu->data;
	int ret = -ENODEV;

	*dev = NULL;

	mdev->mem = dev_get(scfg->mem);
	if(mdev->mem == NULL)
		goto err;

	mdev->mmu = scfg->mmu;

	ret = scpu_register_mem(mdev->mmu->cpu, SPARC_AS_UDATA, &mdev->dev);
	if(ret != 0)
		goto err;

	ret = scpu_register_mem(mdev->mmu->cpu, SPARC_AS_SDATA, &mdev->dev);
	if(ret != 0)
		goto udatarm;

	*dev = &mdev->dev;
	return 0;

udatarm:
	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_UDATA);
err:
	return ret;
}

/**
 * Destroy a sparc fixed mmu data access virtual device
 *
 * @param dev: Device to be removed
 *
 * @return: 0 on success, negative error otherwise
 */
static void snommu_data_destroy(struct dev *dev)
{
	struct snommu_dev *mdev = to_snommu_dev(dev);

	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_UDATA);
	scpu_remove_mem(mdev->mmu->cpu, SPARC_AS_SDATA);
}

/**
 * Create a new sparc fixed mmu device
 *
 * @param dev: Newly created device
 * @param cfg: device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int snommu_create(struct dev **dev, struct devcfg const *cfg)
{
	struct snommu *mmu;
	struct sparc_nommu_cfg const *scfg =
		(struct sparc_nommu_cfg const*)cfg->cfg;
	/* Data mmu device config */
	struct devcfg datacfg = {
		.drvname = "sparc-nommu-data",
		.cfg = DEVCFG(sparc_nommu_dev_cfg) {
			.mem = scfg->dmem,
		},
	};
	/* Device mmu device config */
	struct devcfg isncfg = {
		.drvname = "sparc-nommu-isn",
		.cfg = DEVCFG(sparc_nommu_dev_cfg) {
			.mem = scfg->imem,
		},
	};
	struct dev *d;
	int ret = -ENOMEM;

	*dev = NULL;

	mmu = malloc(sizeof(*mmu));
	if(mmu == NULL)
		goto err;

	ret = -ENODEV;
	mmu->cpu = cpu_get(scfg->cpu);
	if(mmu->cpu == NULL)
		goto err;

	/* Configure data and instruction memory devices */
	ret = -EINVAL;
	if(strlen(cfg->name) + sizeof("-data") > sizeof(datacfg.name))
		goto err;
	strcpy(datacfg.name, cfg->name);
	strcat(datacfg.name, "-data");
	((struct sparc_nommu_dev_cfg *)datacfg.cfg)->mmu = mmu;

	if(strlen(cfg->name) + sizeof("-isn") > sizeof(isncfg.name))
		goto err;
	strcpy(isncfg.name, cfg->name);
	strcat(isncfg.name, "-isn");
	((struct sparc_nommu_dev_cfg *)isncfg.cfg)->mmu = mmu;

	/* Create virtual devices */
	d = dev_create(&datacfg);
	if(d != &mmu->data.dev)
		goto err;

	d = dev_create(&isncfg);
	if(d != &mmu->isn.dev)
		goto derr;

	*dev = &mmu->dev;

	return 0;

derr:
	dev_destroy(&mmu->data.dev);
err:
	if(mmu)
		free(mmu);
	return ret;
}

/**
 * Destroy a sparc fixed mmu device
 *
 * @param dev: To be freed device
 */
static void snommu_destroy(struct dev *dev)
{
	struct snommu *mmu = to_snommu(dev);

	dev_destroy(&mmu->data.dev);
	dev_destroy(&mmu->isn.dev);
	free(mmu);
}

/* Data mmu virtual driver */
static struct devops const snommudops = {
	.create = snommu_data_create,
	.destroy = snommu_data_destroy,
	.read8 = snommu_read8,
	.read16 = snommu_read16,
	.read32 = snommu_read32,
	.write8 = snommu_write8,
	.write16 = snommu_write16,
	.write32 = snommu_write32,
};

static struct drv const snommu_data = {
	.name = "sparc-nommu-data",
	.ops = &snommudops,
};
DRIVER_REGISTER(snommu_data);

/* Instruction mmu virtual driver */
static struct devops const snommuiops = {
	.create = snommu_isn_create,
	.destroy = snommu_isn_destroy,
	.read8 = snommu_fetch_isn8,
	.read16 = snommu_fetch_isn16,
	.read32 = snommu_fetch_isn32,
};

static struct drv const snommu_isn = {
	.name = "sparc-nommu-isn",
	.ops = &snommuiops,
};
DRIVER_REGISTER(snommu_isn);

/* Global mmu driver */
static struct devops const snommuops = {
	.create = snommu_create,
	.destroy = snommu_destroy,
};

static struct drv const snommu = {
	.name = "sparc-nommu",
	.ops = &snommuops,
};
DRIVER_REGISTER(snommu);
