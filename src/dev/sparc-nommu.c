#include <stdlib.h>
#include <errno.h>

#include "types.h"
#include "dev/device.h"
#include "dev/cfg/sparc-nommu.h"
#include "cpu/cpu.h"
#include "cpu/sparc/sparc.h"

struct snommu {
	struct dev dev;
	struct cpu *cpu;
	struct dev *mem;
};
#define to_snommu(d) (container_of(d, struct snommu, dev))

static int snommu_read8(struct dev *dev, addr_t addr, uint8_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->read8)
		return -ENOSYS;
	return mem->drv->ops->read8(mem, addr, val);
}

static int snommu_read16(struct dev *dev, addr_t addr, uint16_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->read16)
		return -ENOSYS;
	return mem->drv->ops->read16(mem, addr, val);
}

static int snommu_read32(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->read32)
		return -ENOSYS;
	return mem->drv->ops->read32(mem, addr, val);
}

static int snommu_write8(struct dev *dev, addr_t addr, uint8_t val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->write8)
		return -ENOSYS;
	return mem->drv->ops->write8(mem, addr, val);
}

static int snommu_write16(struct dev *dev, addr_t addr, uint16_t val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->write16)
		return -ENOSYS;
	return mem->drv->ops->write16(mem, addr, val);
}

static int snommu_write32(struct dev *dev, addr_t addr, uint32_t val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->write32)
		return -ENOSYS;
	return mem->drv->ops->write32(mem, addr, val);
}

static int snommu_fetch_isn8(struct dev *dev, addr_t addr, uint8_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->fetch_isn8)
		return -ENOSYS;
	return mem->drv->ops->fetch_isn8(mem, addr, val);
}

static int snommu_fetch_isn16(struct dev *dev, addr_t addr, uint16_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->fetch_isn16)
		return -ENOSYS;
	return mem->drv->ops->fetch_isn16(mem, addr, val);
}

static int snommu_fetch_isn32(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct dev *mem = to_snommu(dev)->mem;

	if(!mem->drv->ops->fetch_isn32)
		return -ENOSYS;
	return mem->drv->ops->fetch_isn32(mem, addr, val);
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
	int ret = -ENOMEM;

	*dev = NULL;

	mmu = malloc(sizeof(*mmu));
	if(mmu == NULL)
		goto err;

	ret = -ENODEV;
	mmu->mem = dev_get(scfg->mem);
	if(mmu->mem == NULL)
		goto err;

	ret = -ENODEV;
	mmu->cpu = cpu_get(scfg->cpu);
	if(mmu->cpu == NULL)
		goto err;

	ret = scpu_add_memory(mmu->cpu, &mmu->dev);
	if(ret != 0)
		goto err;

	*dev = &mmu->dev;

	return 0;
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

	scpu_rm_memory(mmu->cpu);
	free(mmu);
}

static struct devops const snommuops = {
	.create = snommu_create,
	.destroy = snommu_destroy,
	.read8 = snommu_read8,
	.read16 = snommu_read16,
	.read32 = snommu_read32,
	.write8 = snommu_write8,
	.write16 = snommu_write16,
	.write32 = snommu_write32,
	.fetch_isn8 = snommu_fetch_isn8,
	.fetch_isn16 = snommu_fetch_isn16,
	.fetch_isn32 = snommu_fetch_isn32,
};

static struct drv const snommu = {
	.name = "sparc-nommu",
	.ops = &snommuops,
};

DRIVER_REGISTER(snommu);
