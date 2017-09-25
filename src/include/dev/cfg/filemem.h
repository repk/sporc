#ifndef _DEV_CFG_FILEMEM_H_
#define _DEV_CFG_FILEMEM_H_

/* List of RAM devices configuration */
struct filemem_cfg {
	/* File to map path */
	char *path;
	/* Map start offset */
	off_t off;
	/* Map size */
	size_t sz;
};

#endif
