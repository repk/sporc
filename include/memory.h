#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>
#include <stdint.h>

struct memory;

struct memory_ops {
	struct memory *(*create)(char const *arg);
	void (*destroy)(struct memory *mem);
	int (*map)(struct memory *m, uintptr_t addr, off_t off, size_t sz,
			uint8_t perm);
	int (*unmap)(struct memory *m, uintptr_t addr, size_t sz);
	int (*read8)(struct memory *m, uintptr_t addr, uint8_t *val);
	int (*read16)(struct memory *m, uintptr_t addr, uint16_t *val);
	int (*read32)(struct memory *m, uintptr_t addr, uint32_t *val);
	int (*write8)(struct memory *m, uintptr_t addr, uint8_t val);
	int (*write16)(struct memory *m, uintptr_t addr, uint16_t val);
	int (*write32)(struct memory *m, uintptr_t addr, uint32_t val);
	int (*fetch_isn8)(struct memory *m, uintptr_t addr, uint8_t *val);
	int (*fetch_isn16)(struct memory *m, uintptr_t addr, uint16_t *val);
	int (*fetch_isn32)(struct memory *m, uintptr_t addr, uint32_t *val);
};

struct memory_desc {
	char *name;
	struct memory_ops const *mops;
};

#define MEMORY_REGISTER(m)						\
	__attribute__((section(".rodata.memory"), used))		\
	static struct memory_desc const * const __mem_ ## m = &m

#define MP_R (1 << 1)
#define MP_W (1 << 2)
#define MP_X (1 << 3)

struct memory {
	struct memory_desc const *mem;
	uint8_t perm;
};

struct memory *memory_create(char const *name, char const *args);
void memory_destroy(struct memory *m);
int memory_map(struct memory *m, uintptr_t addr, off_t off, size_t sz,
		uint8_t perm);
int memory_unmap(struct memory *m, uintptr_t addr, size_t sz);
int memory_read8(struct memory *m, uintptr_t addr, uint8_t *val);
int memory_read16(struct memory *m, uintptr_t addr, uint16_t *val);
int memory_read32(struct memory *m, uintptr_t addr, uint32_t *val);
int memory_write8(struct memory *m, uintptr_t addr, uint8_t val);
int memory_write16(struct memory *m, uintptr_t addr, uint16_t val);
int memory_write32(struct memory *m, uintptr_t addr, uint32_t val);
int memory_fetch_isn8(struct memory *m, uintptr_t addr, uint8_t *val);
int memory_fetch_isn16(struct memory *m, uintptr_t addr, uint16_t *val);
int memory_fetch_isn32(struct memory *m, uintptr_t addr, uint32_t *val);

#endif
