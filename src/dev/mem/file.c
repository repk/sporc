#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "utils.h"
#include "types.h"
#include "dev/device.h"
#include "dev/cfg/filemem.h"

#include "ramctl.h"

struct filemem {
	/* Ramctl device */
	struct ramdev ramdev;
	/* Opened file fd */
	int fd;
	/* Actual mapp'ed memory data */
	uint8_t *mapmem;
};
#define to_filemem(m) (container_of(to_ramdev(m), struct filemem, ramdev))

/* Convert a sporc file permission to a POSIX one */
static inline int fmem_perm_flag(perm_t perm)
{
	int flag = -1;

	switch(perm) {
	case MP_W | MP_R:
	case MP_W | MP_R | MP_X:
		flag = O_RDWR;
		break;
	case MP_W :
	case MP_W | MP_X:
		flag = O_WRONLY;
		break;
	case MP_R :
	case MP_R | MP_X:
		flag = O_RDONLY;
		break;
	default:
		break;
	}

	return flag;
}

#define FMEM_BYTE(mem, off) (*((uint8_t *)((mem) + (off))))
#define FMEM_HALF(mem, off) (*((uint16_t *)((mem) + (off))))
#define FMEM_WORD(mem, off) (*((uint32_t *)((mem) + (off))))

/**
 * Fetch a 8 bit value from memory
 */
static int fmem_read8(struct dev *dev, addr_t addr, uint8_t *val)
{
	struct filemem *fdev = to_filemem(dev);

	*val = FMEM_BYTE(fdev->mapmem, addr);

	return 0;
}

/**
 * Fetch a 16 bit value from memory
 */
static int fmem_read16(struct dev *dev, addr_t addr, uint16_t *val)
{
	struct filemem *fdev = to_filemem(dev);

	*val = FMEM_HALF(fdev->mapmem, addr);

	return 0;
}

/**
 * Fetch a 32 bit value from memory
 */
static int fmem_read32(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct filemem *fdev = to_filemem(dev);

	*val = FMEM_WORD(fdev->mapmem, addr);

	return 0;
}

/**
 * Write a 8 bit value into memory
 */
static int fmem_write8(struct dev *dev, addr_t addr, uint8_t val)
{
	struct filemem *fdev = to_filemem(dev);

	FMEM_BYTE(fdev->mapmem, addr) = val;

	return 0;
}

/**
 * Write a 16 bit value into memory
 */
static int fmem_write16(struct dev *dev, addr_t addr, uint16_t val)
{
	struct filemem *fdev = to_filemem(dev);

	FMEM_HALF(fdev->mapmem, addr) = val;

	return 0;
}

/**
 * Write a 32 bit value into memory
 */
static int fmem_write32(struct dev *dev, addr_t addr, uint32_t val)
{
	struct filemem *fdev = to_filemem(dev);

	FMEM_WORD(fdev->mapmem, addr) = val;

	return 0;
}

/**
 * Map a file to memory
 */
static inline int fmem_map(struct filemem *fmem, off_t off)
{
	int prot = PROT_NONE;

	if(fmem->ramdev.perm & MP_R)
		prot |= PROT_READ;
	if(fmem->ramdev.perm & MP_W)
		prot |= PROT_WRITE;

	fmem->mapmem = mmap(NULL, fmem->ramdev.size, prot, MAP_PRIVATE,
			fmem->fd, off);
	if(fmem->mapmem == MAP_FAILED)
		return -ENOMEM;

	return 0;
}

/**
 * Unmap a mapped file
 */
static inline int fmem_unmap(struct filemem *fmem)
{
	munmap(fmem->mapmem, fmem->ramdev.size);
	fmem->mapmem = NULL;

	return 0;
}

/**
 * Create a new file memory mapping device instance
 *
 * This will try to map file to memory with the highest permissions possible
 */
static int fmem_create(struct dev **dev, struct devcfg const *cfg)
{
	struct filemem *fm = NULL;
	struct filemem_cfg const *fcfg = (struct filemem_cfg const *)cfg->cfg;
	struct stat st;
	size_t i, sz;
	int err, flag, fd = -1;
	static perm_t const perm[] = {
		MP_R | MP_W | MP_X,
		MP_W,
		MP_R | MP_X,
	};

	/* Try to open file with the most permissive rigths possible (R|W) */
	for(i = 0; i < ARRAY_SIZE(perm); ++i) {
		/* Get POSIX permission flag */
		flag = fmem_perm_flag(perm[i]);
		if(flag < 0) {
			ERR("Invalid perm 0x%x\n", perm[i]);
			err = -EINVAL;
			goto exit;
		}

		fd = open(fcfg->path, flag);
		if((fd < 0) && (errno != EACCES)) {
			PERR("Cannot open %s", fcfg->path);
			err = errno;
			goto exit;
		}

		/* open succeed */
		if(fd >= 0)
			break;
	}
	if(fd < 0) {
		PERR("Cannot open %s", fcfg->path);
		err = -EACCES;
		goto exit;
	}

	/* Find file size */
	err = fstat(fd, &st);
	if(err != 0) {
		PERR("Cannot stat %s\n", fcfg->path);
		close(fd);
		goto exit;
	}

	/* Get map size from config. If its -1 map to the whole file size */
	sz = fcfg->sz;
	if(sz == (size_t)(-1))
		sz = st.st_size;

	fm = calloc(1, sizeof(*fm));
	if(fm == NULL) {
		close(fd);
		err = ENOMEM;
		goto exit;
	}

	fm->fd = fd;
	fm->ramdev.perm = perm[i];
	fm->ramdev.size = sz;
	*dev = &fm->ramdev.dev;

	/* Map file to memory */
	err = fmem_map(fm, fcfg->off);
	if(err != 0) {
		close(fd);
		free(fm);
		err = ENOMEM;
		goto exit;
	}

exit:
	return err;
}

/*
 * Destroy a file memory mapping device instance
 */
static void fmem_destroy(struct dev *dev)
{
	struct filemem *fm = to_filemem(dev);

	fmem_unmap(fm);
	close(fm->fd);
	free(fm);
}

/*
 * File memory mapping driver operations
 */
static struct devops const fmops = {
	.create = fmem_create,
	.destroy = fmem_destroy,
	.read8 = fmem_read8,
	.read16 = fmem_read16,
	.read32 = fmem_read32,
	.write8 = fmem_write8,
	.write16 = fmem_write16,
	.write32 = fmem_write32,
	.fetch_isn8 = fmem_read8,
	.fetch_isn16 = fmem_read16,
	.fetch_isn32 = fmem_read32,
};

/*
 * File memory driver structure
 */
static struct drv const fmem = {
	.name = "file-mem",
	.ops = &fmops,
};

DRIVER_REGISTER(fmem);
