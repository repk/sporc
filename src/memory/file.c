#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "utils.h"

#include "memory.h"

#define FMEM_MAPSZ 16
struct fmem_map_addr {
	/* Actual mapp'ed memory data */
	uint8_t *mapmem;
	/* Virtual map address */
	uintptr_t addr;
	/* Size of map */
	size_t size;
	/* Offset in opened file */
	off_t off;
	/* Permission for this particular map chunk */
	uint8_t perm;
};

struct filemem {
	struct memory mem;
	/* Memory map array */
	struct fmem_map_addr maddr[FMEM_MAPSZ];
	/* Size of the opened file */
	size_t size;
	/* Opened file fd */
	int fd;
	/* Opened file most permissive rights */
	uint8_t perm;
};

#define to_filemem(m) (container_of(m, struct filemem, mem))

/* Convert a sporc file permission to a POSIX one */
static inline int fmem_perm_flag(uint8_t perm)
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

/* Find a map in plugin's map array */
static inline struct fmem_map_addr *fmem_get_map(struct memory *mem,
		uintptr_t addr, size_t sz)
{
	struct filemem *fm = to_filemem(mem);
	size_t i;

	for(i = 0; i < ARRAY_SIZE(fm->maddr); ++i) {
		if((addr >= fm->maddr[i].addr) &&
				(addr + sz <= fm->maddr[i].addr +
				 fm->maddr[i].size) &&
				(fm->maddr[i].mapmem != NULL))
			break;
	}

	if(i == ARRAY_SIZE(fm->maddr))
		return NULL;

	return &fm->maddr[i];
}

static int fmem_read8(struct memory *mem, uintptr_t addr, uint8_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & MP_R))
		return -EACCES;

	*val = FMEM_BYTE(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_read16(struct memory *mem, uintptr_t addr, uint16_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & MP_R))
		return -EACCES;

	*val = FMEM_HALF(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_read32(struct memory *mem, uintptr_t addr, uint32_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & MP_R))
		return -EACCES;

	*val = FMEM_WORD(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_write8(struct memory *mem, uintptr_t addr, uint8_t val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(val));

	if((maddr == NULL) || !(maddr->perm & MP_W))
		return -EACCES;

	FMEM_BYTE(maddr->mapmem, addr - maddr->addr) = val;

	return 0;
}

static int fmem_write16(struct memory *mem, uintptr_t addr, uint16_t val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(val));

	if((maddr == NULL) || !(maddr->perm & MP_W))
		return -EACCES;

	FMEM_HALF(maddr->mapmem, addr - maddr->addr) = val;

	return 0;
}

static int fmem_write32(struct memory *mem, uintptr_t addr, uint32_t val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(val));

	if((maddr == NULL) || !(maddr->perm & MP_W))
		return -EACCES;

	FMEM_WORD(maddr->mapmem, addr - maddr->addr) = val;

	return 0;
}

static int fmem_fetch_isn8(struct memory *mem, uintptr_t addr, uint8_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & (MP_R | MP_X)))
		return -EACCES;

	*val = FMEM_BYTE(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_fetch_isn16(struct memory *mem, uintptr_t addr, uint16_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & (MP_R | MP_X)))
		return -EACCES;

	*val = FMEM_HALF(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_fetch_isn32(struct memory *mem, uintptr_t addr, uint32_t *val)
{
	struct fmem_map_addr *maddr = fmem_get_map(mem, addr, sizeof(*val));

	if((maddr == NULL) || !(maddr->perm & (MP_R | MP_X)))
		return -EACCES;

	*val = FMEM_WORD(maddr->mapmem, addr - maddr->addr);

	return 0;
}

static int fmem_map(struct memory *mem, uintptr_t addr, off_t off, size_t sz,
		uint8_t perm)
{
	struct filemem *fmem = to_filemem(mem);
	struct fmem_map_addr *fa;
	void *memptr;
	size_t i;
	int prot = PROT_NONE;

	if(((perm & ~MP_X) | fmem->perm) != fmem->perm)
		return -EACCES;

	/* TODO do not map already mapped file */
	for(i = 0; i < ARRAY_SIZE(fmem->maddr); ++i)
		if(fmem->maddr[i].mapmem == NULL)
			break;

	if(i == ARRAY_SIZE(fmem->maddr))
		return -ENOMEM;

	fa = &fmem->maddr[i];

	if(perm & MP_R)
		prot |= PROT_READ;
	if(perm & MP_W)
		prot |= PROT_WRITE;

	if(sz == (size_t)(-1))
		sz = fmem->size;

	memptr = mmap(NULL, sz, prot, MAP_PRIVATE, fmem->fd, off);
	if(memptr == MAP_FAILED)
		return -ENOMEM;

	fa->mapmem = memptr;
	fa->addr = addr;
	fa->size = sz;
	fa->perm = perm;
	fa->off = off;

	return 0;
}

static int fmem_unmap(struct memory *mem, uintptr_t addr, size_t sz)
{
	struct filemem *fm = to_filemem(mem);
	size_t i;

	/* Find the requested map in the map array */
	for(i = 0; i < ARRAY_SIZE(fm->maddr); ++i)
		if((fm->maddr[i].addr == addr) &&
				((fm->maddr[i].size == sz) ||
				 (sz == (size_t)(-1))))
			break;

	if(i == ARRAY_SIZE(fm->maddr))
		return -EINVAL;

	munmap(fm->maddr[i].mapmem, fm->maddr[i].size);
	fm->maddr[i].mapmem = NULL;

	return 0;
}

static struct memory *fmem_create(char const *file)
{
	struct filemem *fm = NULL;
	struct memory *ret = NULL;
	struct stat st;
	int err, flag, fd;
	uint8_t perm = 0;

	/* Try to mmap file with more permissive rights possible (R|W) */
	err = access(file, R_OK);
	if(err == 0)
		perm |= MP_R;

	err = access(file, W_OK);
	if(err == 0)
		perm |= MP_W;

	/* Find file size */
	err = stat(file, &st);
	if(err != 0) {
		PERR("Cannot stat %s\n", file);
		goto exit;
	}

	/* Get POSIX permission flag */
	flag = fmem_perm_flag(perm);
	if(flag < 0) {
		ERR("Invalid perm 0x%x\n", perm);
		goto exit;
	}

	fd = open(file, flag);
	if(fd < 0) {
		PERR("Cannot open %s", file);
		goto exit;
	}

	fm = calloc(1, sizeof(*fm));
	if(fm == NULL) {
		close(fd);
		goto exit;
	}

	fm->fd = fd;
	fm->perm = perm;
	fm->size = st.st_size;
	ret = &fm->mem;

exit:
	return ret;
}

static void fmem_destroy(struct memory *mem)
{
	struct filemem *fm = to_filemem(mem);

	close(fm->fd);
	free(fm);
}

static struct memory_ops const fmops = {
	.create = fmem_create,
	.destroy = fmem_destroy,
	.map = fmem_map,
	.unmap = fmem_unmap,
	.read8 = fmem_read8,
	.read16 = fmem_read16,
	.read32 = fmem_read32,
	.write8 = fmem_write8,
	.write16 = fmem_write16,
	.write32 = fmem_write32,
	.fetch_isn8 = fmem_fetch_isn8,
	.fetch_isn16 = fmem_fetch_isn16,
	.fetch_isn32 = fmem_fetch_isn32,
};

static struct memory_desc const fmem = {
	.name = "file-mem",
	.mops = &fmops,
};

MEMORY_REGISTER(fmem);
