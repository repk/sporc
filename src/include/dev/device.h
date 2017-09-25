#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <errno.h>

#include "list.h"
#include "types.h"

#define DEVNAMESZ 64

/**
 * Device configuration
 */
struct devcfg {
	/* Controlling driver name */
	char *drvname;
	/* Device unique name */
	char name[DEVNAMESZ];
	/* Device specific configuration */
	void *cfg;
};
#define DEVCFG(n) &(struct n)

struct dev;

/**
 * Device operations
 */
struct devops {
	/**
	 * Create a new device instance
	 */
	int (*create)(struct dev **dev, struct devcfg const *cfg);
	/**
	 * Destroy a device
	 */
	void (*destroy)(struct dev *dev);
	/**
	 * Read device's memory/register
	 */
	int (*read8)(struct dev *dev, addr_t addr, uint8_t *val);
	int (*read16)(struct dev *dev, addr_t addr, uint16_t *val);
	int (*read32)(struct dev *dev, addr_t addr, uint32_t *val);
	/**
	 * Write into device's memory/register
	 */
	int (*write8)(struct dev *dev, addr_t addr, uint8_t val);
	int (*write16)(struct dev *dev, addr_t addr, uint16_t val);
	int (*write32)(struct dev *dev, addr_t addr, uint32_t val);
	/**
	 * Fetch with exec perm device's memory
	 */
	int (*fetch_isn8)(struct dev *dev, addr_t addr, uint8_t *val);
	int (*fetch_isn16)(struct dev *dev, addr_t addr, uint16_t *val);
	int (*fetch_isn32)(struct dev *dev, addr_t addr, uint32_t *val);
};

/**
 * Driver description
 */
struct drv {
	/**
	 * Driver name
	 */
	char const *name;
	/**
	 * Driver operations
	 */
	struct devops const *ops;
};

/**
 * Device structure
 */
struct dev {
	/**
	 * Next device in whole system device linked list
	 */
	struct list_head next;
	/**
	 * Device controlling driver
	 */
	struct drv const *drv;
	/**
	 * Unique device name
	 */
	char name[DEVNAMESZ];
};

struct dev *dev_get(char const *name);
struct dev *dev_create(struct devcfg const *cfg);
void dev_destroy(struct dev *d);

static inline int dev_read8(struct dev *dev, addr_t addr, uint8_t *val)
{
	if(!dev->drv->ops->read8)
		return -ENOSYS;
	return dev->drv->ops->read8(dev, addr, val);
}

static inline int dev_read16(struct dev *dev, addr_t addr, uint16_t *val)
{
	if(!dev->drv->ops->read16)
		return -ENOSYS;
	return dev->drv->ops->read16(dev, addr, val);
}

static inline int dev_read32(struct dev *dev, addr_t addr, uint32_t *val)
{
	if(!dev->drv->ops->read32)
		return -ENOSYS;
	return dev->drv->ops->read32(dev, addr, val);
}

static inline int dev_write8(struct dev *dev, addr_t addr, uint8_t val)
{
	if(!dev->drv->ops->write8)
		return -ENOSYS;
	return dev->drv->ops->write8(dev, addr, val);
}

static inline int dev_write16(struct dev *dev, addr_t addr, uint16_t val)
{
	if(!dev->drv->ops->write16)
		return -ENOSYS;
	return dev->drv->ops->write16(dev, addr, val);
}

static inline int dev_write32(struct dev *dev, addr_t addr, uint32_t val)
{
	if(!dev->drv->ops->write32)
		return -ENOSYS;
	return dev->drv->ops->write32(dev, addr, val);
}

static inline int dev_fetch_isn8(struct dev *dev, addr_t addr, uint8_t *val)
{
	if(!dev->drv->ops->fetch_isn8)
		return -ENOSYS;
	return dev->drv->ops->fetch_isn8(dev, addr, val);
}

static inline int dev_fetch_isn16(struct dev *dev, addr_t addr, uint16_t *val)
{
	if(!dev->drv->ops->fetch_isn16)
		return -ENOSYS;
	return dev->drv->ops->fetch_isn16(dev, addr, val);
}

static inline int dev_fetch_isn32(struct dev *dev, addr_t addr, uint32_t *val)
{
	if(!dev->drv->ops->fetch_isn32)
		return -ENOSYS;
	return dev->drv->ops->fetch_isn32(dev, addr, val);
}

/**
 * Register a memory plugin
 */
#define DRIVER_REGISTER(d)						\
	__attribute__((section(".rodata.driver"), used))		\
	static struct drv const * const __drv_ ## d = &d


#endif
