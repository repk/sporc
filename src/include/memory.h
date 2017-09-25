#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>
#include <stdint.h>

#include "types.h"

struct memory;

/**
 * Memory plugin operations
 */
struct memory_ops {
	/**
	 * Plugin instantiation
	 */
	struct memory *(*create)(char const *arg);
	/**
	 * Plugin instance destruction
	 */
	void (*destroy)(struct memory *mem);
	/**
	 * Map a bunch of memory
	 */
	int (*map)(struct memory *m, uintptr_t addr, off_t off, size_t sz,
			perm_t perm);
	/**
	 * Unmap a mapp'ed memory
	 */
	int (*unmap)(struct memory *m, uintptr_t addr, size_t sz);
	/**
	 * Read one byte
	 */
	int (*read8)(struct memory *m, uintptr_t addr, uint8_t *val);
	/**
	 * Read one halfword
	 */
	int (*read16)(struct memory *m, uintptr_t addr, uint16_t *val);
	/**
	 * Read one word
	 */
	int (*read32)(struct memory *m, uintptr_t addr, uint32_t *val);
	/**
	 * Write one byte
	 */
	int (*write8)(struct memory *m, uintptr_t addr, uint8_t val);
	/**
	 * Write one halfword
	 */
	int (*write16)(struct memory *m, uintptr_t addr, uint16_t val);
	/**
	 * Write one word
	 */
	int (*write32)(struct memory *m, uintptr_t addr, uint32_t val);
	/**
	 * Fetch one byte for exec
	 */
	int (*fetch_isn8)(struct memory *m, uintptr_t addr, uint8_t *val);
	/**
	 * Fetch one halfword for exec
	 */
	int (*fetch_isn16)(struct memory *m, uintptr_t addr, uint16_t *val);
	/**
	 * Fetch one word for exec
	 */
	int (*fetch_isn32)(struct memory *m, uintptr_t addr, uint32_t *val);
};

/**
 * A memory plugin descriptor
 */
struct memory_desc {
	/**
	 * Name of memory plugin used as id
	 */
	char *name;
	/**
	 * Plugin operations
	 */
	struct memory_ops const *mops;
};

/**
 * Register a memory plugin
 */
#define MEMORY_REGISTER(m)						\
	__attribute__((section(".rodata.memory"), used))		\
	static struct memory_desc const * const __mem_ ## m = &m

/**
 * Read permission flag
 */
#define MP_R (1 << 1)
/**
 * write permission flag
 */
#define MP_W (1 << 2)
/**
 * exec permission flag
 */
#define MP_X (1 << 3)

/**
 * Memory instance structure
 */
struct memory {
	/**
	 * Plugin descriptor
	 */
	struct memory_desc const *mem;
};

struct memory *memory_create(char const *name, char const *args);
void memory_destroy(struct memory *m);
int memory_map(struct memory *m, uintptr_t addr, off_t off, size_t sz,
		perm_t perm);
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
