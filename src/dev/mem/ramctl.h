#ifndef _DEV_MEM_RAMCTL_H_
#define _DEV_MEM_RAMCTL_H_

struct ramdev {
	/* Device that handles access to this memory map */
	struct dev dev;
	/* Size of map */
	size_t size;
	/* Memory access rights */
	perm_t perm;
};
#define to_ramdev(d) (container_of(d, struct ramdev, dev))

#endif
