#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "types.h"
#include "dev/device.h"
#include "dev/cfg/ramctl.h"

#include "ramctl.h"

#define RAMMAP_MAX 16

struct ramdev_map {
	/* Memory device */
	struct ramdev *dev;
	/* Physical map address */
	phyaddr_t addr;
	/* Permission for this map chunk */
	perm_t perm;
};

struct ramctl {
	/* Ram device */
	struct dev dev;
	/* Ram address maping to device array */
	struct ramdev_map map[RAMMAP_MAX];
};
#define to_ramctl(d) (container_of(d, struct ramctl, dev))

static inline struct ramdev_map *ramctl_get_map(struct dev *dev,
		phyaddr_t addr, size_t sz)
{
	struct ramdev_map *map = to_ramctl(dev)->map;
	struct ramdev_map *ret = NULL;
	size_t i;

	for(i = 0; i < ARRAY_SIZE(to_ramctl(dev)->map); ++i) {
		if(map[i].dev && map[i].addr <= addr &&
				map[i].addr + map[i].dev->size >= addr + sz)
		{
			ret = &map[i];
			break;
		}
	}

	return ret;
}

static int ramctl_read8(struct dev *dev, phyaddr_t addr, uint8_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 1);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->read8)
		return -ENOSYS;

	if(!(rd->perm & MP_R))
		return -EACCES;

	return mapdev->drv->phyops->read8(mapdev, addr - rd->addr, val);
}

static int ramctl_read16(struct dev *dev, phyaddr_t addr, uint16_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 2);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->read16)
		return -ENOSYS;

	if(!(rd->perm & MP_R))
		return -EACCES;

	return mapdev->drv->phyops->read16(mapdev, addr - rd->addr, val);
}

static int ramctl_read32(struct dev *dev, phyaddr_t addr, uint32_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 4);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->read32)
		return -ENOSYS;

	if(!(rd->perm & MP_R))
		return -EACCES;

	return mapdev->drv->phyops->read32(mapdev, addr - rd->addr, val);
}

static int ramctl_write8(struct dev *dev, phyaddr_t addr, uint8_t val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 1);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->write8)
		return -ENOSYS;

	if(!(rd->perm & MP_W))
		return -EACCES;

	return mapdev->drv->phyops->write8(mapdev, addr - rd->addr, val);
}

static int ramctl_write16(struct dev *dev, phyaddr_t addr, uint16_t val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 2);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->write16)
		return -ENOSYS;

	if(!(rd->perm & MP_W))
		return -EACCES;

	return mapdev->drv->phyops->write16(mapdev, addr - rd->addr, val);
}

static int ramctl_write32(struct dev *dev, phyaddr_t addr, uint32_t val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 4);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->write32)
		return -ENOSYS;

	if(!(rd->perm & MP_W))
		return -EACCES;

	return mapdev->drv->phyops->write32(mapdev, addr - rd->addr, val);
}

static int ramctl_fetch_isn8(struct dev *dev, phyaddr_t addr, uint8_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 1);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->fetch_isn8)
		return -ENOSYS;

	if(!(rd->perm & MP_X))
		return -EACCES;

	return mapdev->drv->phyops->fetch_isn8(mapdev, addr - rd->addr, val);
}

static int ramctl_fetch_isn16(struct dev *dev, phyaddr_t addr, uint16_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 2);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->fetch_isn16)
		return -ENOSYS;

	if(!(rd->perm & MP_X))
		return -EACCES;

	return mapdev->drv->phyops->fetch_isn16(mapdev, addr - rd->addr, val);
}

static int ramctl_fetch_isn32(struct dev *dev, phyaddr_t addr, uint32_t *val)
{
	struct ramdev_map *rd = ramctl_get_map(dev, addr, 4);
	struct dev *mapdev;

	if(rd == NULL)
		return -ENODEV;

	mapdev = &rd->dev->dev;

	if(!mapdev->drv->phyops->fetch_isn32)
		return -ENOSYS;

	if(!(rd->perm & MP_X))
		return -EACCES;

	return mapdev->drv->phyops->fetch_isn32(mapdev, addr - rd->addr, val);
}

static int ram_map(struct ramctl *ctl, struct rammap const *map)
{
	struct ramdev *mapdev;
	struct dev *dev;
	size_t i;

	/* Get map'ed memory handling device */
	dev = dev_get(map->devname);
	if(dev == NULL)
		return -ENODEV;

	/* Check device access permissions */
	mapdev = to_ramdev(dev);
	if((mapdev->perm & map->perm) != map->perm)
		return -EACCES;

	/* Find an empty map */
	for(i = 0; i < ARRAY_SIZE(ctl->map); ++i)
		if(ctl->map[i].dev == NULL)
			break;

	/* No more free map */
	if(i == ARRAY_SIZE(ctl->map))
		return -ENOMEM;

	/* Fill ram device map infos */
	ctl->map[i].dev = mapdev;
	ctl->map[i].addr = map->addr;
	ctl->map[i].perm = map->perm;

	return 0;
}

static void ram_unmap(struct ramctl *ctl, size_t mapid)
{
	ctl->map[mapid].dev = NULL;
}

/**
 * Create a new RAM controller device
 *
 * @param dev: Newly created device
 * @param cfg: device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int ramctl_create(struct dev **dev, struct devcfg const *cfg)
{
	struct ramctl *ctl;
	struct ramctl_cfg const *rcfg = (struct ramctl_cfg const*)cfg->cfg;
	struct rammap *map;
	int ret = -ENOMEM;

	*dev = NULL;

	ctl = calloc(1, sizeof(*ctl));
	if(ctl == NULL)
		goto err;

	for(map = rcfg->devlst; map->devname != NULL; ++map) {
		ret = ram_map(ctl, map);
		if(ret != 0)
			goto err;
	}

	*dev = &ctl->dev;

	return 0;
err:
	if(ctl)
		free(ctl);
	return ret;
}

/**
 * Destroy a RAM controller device
 *
 * @param dev: To be freed device
 */
static void ramctl_destroy(struct dev *dev)
{
	struct ramctl *ctl = to_ramctl(dev);
	size_t i;

	for(i = 0; i < ARRAY_SIZE(ctl->map); ++i)
		if(ctl->map[i].dev == NULL)
			ram_unmap(ctl, i);

	free(ctl);
}

static struct phydevops const ramctlops = {
	.create = ramctl_create,
	.destroy = ramctl_destroy,
	.read8 = ramctl_read8,
	.read16 = ramctl_read16,
	.read32 = ramctl_read32,
	.write8 = ramctl_write8,
	.write16 = ramctl_write16,
	.write32 = ramctl_write32,
	.fetch_isn8 = ramctl_fetch_isn8,
	.fetch_isn16 = ramctl_fetch_isn16,
	.fetch_isn32 = ramctl_fetch_isn32,
};

static struct drv const ram = {
	.name = "ramctl",
	.phyops = &ramctlops,
};

DRIVER_REGISTER(ram);
