#ifndef _DEV_CFG_RAMCTL_H_
#define _DEV_CFG_RAMCTL_H_

/* RAM device mapping config */
struct rammap {
	/* Name of device that handles memory accesses for this map */
	char const *devname;
	/* Map address */
	addr_t addr;
	/* Permission for memory accesses */
	perm_t perm;
	/* Size of the memory map */
	size_t sz;
};

/* List of RAM devices configuration */
struct ramctl_cfg {
	struct rammap *devlst;
};

#endif
