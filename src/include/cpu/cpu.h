#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

#include "types.h"

struct cpu_desc;

/**
 * Cpu operations
 */
struct cpu_ops {
	/**
	 * Plugin instantiation
	 */
	struct cpu *(*create)(char const *args);
	/**
	 * Plugin instance destruction
	 */
	void (*destroy)(struct cpu *cpu);
	/**
	 * Boot this cpu at specific address
	 */
	int (*boot)(struct cpu *cpu, addr_t addr);

	/* TODO addInterrupt */

	/**
	 * Instruction fetch operation
	 */
	int (*fetch)(struct cpu *cpu);
	/**
	 * Instruction decode operation
	 */
	int (*decode)(struct cpu *cpu);
	/**
	 * Instruction execution operation
	 */
	int (*exec)(struct cpu *cpu);
};

/**
 * Cpu plugin descriptor
 */
struct cpu_desc {
	/**
	 * Cpu plugin name used as id
	 */
	char *name;
	/**
	 * Plugin operations
	 */
	struct cpu_ops const *cops;
};

/**
 * Cpu instance structure
 */
struct cpu {
	/**
	 * Plugin descriptor
	 */
	struct cpu_desc const *cpu;
	/**
	 * Cpu memory map
	 */
	struct memory *mem;
};

/**
 * Register a cpu plugin
 */
#define CPU_REGISTER(c)							\
	__attribute__((section(".rodata.cpu"), used))			\
	static struct cpu_desc const * const __cpu_ ## c = &c

int cpu_fetch(struct cpu *c);
int cpu_decode(struct cpu *c);
int cpu_exec(struct cpu *c);
int cpu_boot(struct cpu *c, addr_t addr);
struct cpu *cpu_create(char const *name, struct memory *mem, char const *args);
int cpu_destroy(struct cpu *c);

#endif
