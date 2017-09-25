#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "dev/device.h"

static LIST_HEAD(devlst);

/**
 * Find a registered driver from its name.
 *
 * @param name: Name of requested driver
 * @return: The device driver descriptor if found, NULL pointer otherwise
 */
static struct drv const *drv_get(char const *name)
{
	struct drv const * const *d;
	struct drv const *ret = NULL;
	extern struct drv const *__drv_start;
	extern struct drv const *__drv_end;

	for(d = &__drv_start; d < &__drv_end; ++d) {
		if(strcmp((*d)->name, name) == 0) {
			ret = *d;
			break;
		}
	}

	return ret;
}

/**
 * Find device from its name.
 *
 * @param name: Name of requested device.
 * @return: The device structure if found, NULL pointer otherwise
 */
struct dev *dev_get(char const *name)
{
	struct dev *p;

	/* TODO Lock the list if SMP */
	list_for_each_entry(p, &devlst, next)
		if(strcmp(p->name, name) == 0)
			goto out;

	p = NULL;
out:
	return p;
}

/**
 * Instantiate a device from its driver.
 *
 * @param dev: Device configuration (Driver name, dev name, specific cfg)
 * @return: NULL pointer on error, a device plugin instance otherwise
 */
struct dev *dev_create(struct devcfg const *cfg)
{
	struct drv const *drv;
	struct dev *dev, *d = NULL;
	int ret;

	drv = drv_get(cfg->drvname);
	if(drv == NULL)
		goto out;

	ret = drv->ops->create(&dev, cfg);
	if(ret != 0)
		goto out;

	d = dev;
	d->drv = drv;
	strcpy(d->name, cfg->name);
	/* TODO Lock the list if SMP */
	list_add(&d->next, &devlst);

out:
	return d;
}

/**
 * Destroy a device plugin instance.
 *
 * @param d: device instance.
 */
void dev_destroy(struct dev *d)
{
	/* TODO Lock the list if SMP */
	list_del(&d->next);
	d->drv->ops->destroy(d);
}
