#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "memory.h"

/**
 * Find a registered memory plugin from its name.
 *
 * @param name: Name of requested memory plugin
 * @return: The memory plugin descriptor if found, NULL pointer otherwise
 */
static struct memory_desc const *memory_desc_get(char const *name)
{
	struct memory_desc const * const *m;
	struct memory_desc const *ret = NULL;
	extern struct memory_desc const *__memory_start;
	extern struct memory_desc const *__memory_end;

	for(m = &__memory_start; m < &__memory_end; ++m) {
		if(strcmp((*m)->name, name) == 0) {
			ret = *m;
			break;
		}
	}

	return ret;
}

/**
 * Instantiate a memory plugin subsystem.
 *
 * @param name: Name of requested memory plugin
 * @param args: Memory plugin specific arguments
 * @return: NULL pointer on error, a memory plugin instance otherwise
 */
struct memory *memory_create(char const *name, char const *args)
{
	struct memory_desc const *mdesc;
	struct memory *mem;

	mdesc = memory_desc_get(name);
	if(mdesc == NULL)
		return NULL;

	mem = mdesc->mops->create(args);
	if(mem == NULL)
		return NULL;

	mem->mem = mdesc;
	return mem;
}

/**
 * Destroy a memory plugin instance.
 *
 * @param m: plugin instance.
 */
void memory_destroy(struct memory *m)
{
	m->mem->mops->destroy(m);
}

/**
 * Map a memory address space through a specific plugin
 *
 * @param m: memory plugin instance.
 * @param addr: where to map the memory space
 * @param off: start offset of memory space to map
 * @param sz: size of memory space to map
 * @param perm: memory map permission (R0, RW, ...)
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_map(struct memory *m, uintptr_t addr, off_t off, size_t sz,
		perm_t perm)
{
	return m->mem->mops->map(m, addr, off, sz, perm);
}

/**
 * Unmap a chunk of mapp'ed memory
 *
 * @param m: memory plugin instance.
 * @param addr: where to unmap the memory space
 * @param sz: size of memory space to unmap
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_unmap(struct memory *m, uintptr_t addr, size_t sz)
{
	return m->mem->mops->unmap(m, addr, sz);
}

/**
 * Fetch a byte in a mapp'ed memory (read)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched byte
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_read8(struct memory *m, uintptr_t addr, uint8_t *val)
{
	return m->mem->mops->read8(m, addr, val);
}

/**
 * Fetch a halfword in a mapp'ed memory (read)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched halfword
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_read16(struct memory *m, uintptr_t addr, uint16_t *val)
{
	return m->mem->mops->read16(m, addr, val);
}

/**
 * Fetch a word in a mapp'ed memory (read)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched word
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_read32(struct memory *m, uintptr_t addr, uint32_t *val)
{
	return m->mem->mops->read32(m, addr, val);
}

/**
 * Write a byte in a mapp'ed memory (write)
 *
 * @param m: memory plugin instance.
 * @param addr: where to write in memory
 * @param val: value to write in memory
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_write8(struct memory *m, uintptr_t addr, uint8_t val)
{
	return m->mem->mops->write8(m, addr, val);
}

/**
 * Write a halfbyte in a mapp'ed memory (write)
 *
 * @param m: memory plugin instance.
 * @param addr: where to write in memory
 * @param val: value to write in memory
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_write16(struct memory *m, uintptr_t addr, uint16_t val)
{
	return m->mem->mops->write16(m, addr, val);
}

/**
 * Write a word in a mapp'ed memory (write)
 *
 * @param m: memory plugin instance.
 * @param addr: where to write in memory
 * @param val: value to write in memory
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_write32(struct memory *m, uintptr_t addr, uint32_t val)
{
	return m->mem->mops->write32(m, addr, val);
}

/**
 * Fetch a byte instruction in a mapp'ed memory (exec)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched byte
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_fetch_isn8(struct memory *m, uintptr_t addr, uint8_t *val)
{
	return m->mem->mops->fetch_isn8(m, addr, val);
}

/**
 * Fetch a halfbyte instruction in a mapp'ed memory (exec)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched halfbyte
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_fetch_isn16(struct memory *m, uintptr_t addr, uint16_t *val)
{
	return m->mem->mops->fetch_isn16(m, addr, val);
}

/**
 * Fetch a word instruction in a mapp'ed memory (exec)
 *
 * @param m: memory plugin instance.
 * @param addr: where to fetch
 * @param val: filled with the fetched word
 *
 * @return: 0 on success, negative number otherwise
 */
int memory_fetch_isn32(struct memory *m, uintptr_t addr, uint32_t *val)
{
	return m->mem->mops->fetch_isn32(m, addr, val);
}
