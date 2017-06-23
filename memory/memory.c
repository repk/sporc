#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "memory.h"

static struct memory_desc const *memory_desc_get(char const *name)
{
	struct memory_desc const * const *m;
	struct memory_desc const *ret;
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

void memory_destroy(struct memory *m)
{
	m->mem->mops->destroy(m);
}

int memory_map(struct memory *m, uintptr_t addr, off_t off, size_t sz,
		uint8_t perm)
{
	return m->mem->mops->map(m, addr, off, sz, perm);
}

int memory_unmap(struct memory *m, uintptr_t addr, size_t sz)
{
	return m->mem->mops->unmap(m, addr, sz);
}

int memory_read8(struct memory *m, uintptr_t addr, uint8_t *val)
{
	return m->mem->mops->read8(m, addr, val);
}

int memory_read16(struct memory *m, uintptr_t addr, uint16_t *val)
{
	return m->mem->mops->read16(m, addr, val);
}

int memory_read32(struct memory *m, uintptr_t addr, uint32_t *val)
{
	return m->mem->mops->read32(m, addr, val);
}

int memory_write8(struct memory *m, uintptr_t addr, uint8_t val)
{
	return m->mem->mops->write8(m, addr, val);
}

int memory_write16(struct memory *m, uintptr_t addr, uint16_t val)
{
	return m->mem->mops->write16(m, addr, val);
}

int memory_write32(struct memory *m, uintptr_t addr, uint32_t val)
{
	return m->mem->mops->write32(m, addr, val);
}

int memory_fetch_isn8(struct memory *m, uintptr_t addr, uint8_t *val)
{
	return m->mem->mops->fetch_isn8(m, addr, val);
}

int memory_fetch_isn16(struct memory *m, uintptr_t addr, uint16_t *val)
{
	return m->mem->mops->fetch_isn16(m, addr, val);
}

int memory_fetch_isn32(struct memory *m, uintptr_t addr, uint32_t *val)
{
	return m->mem->mops->fetch_isn32(m, addr, val);
}
