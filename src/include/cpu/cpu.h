#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

#include "types.h"
#include "list.h"

#include "dev/device.h"

#define CPUNAMESZ 64

/**
 * Cpu configuration
 */
struct cpucfg {
	/* Name of cpu driver */
	char const *cpu;
	/* Name of cpu instance */
	char const name[CPUNAMESZ];
	/* Cpu specific configuration */
	void *cfg;
};
#define CPUCFG(n) &(struct n)

/**
 * Cpu operations
 */
struct cpu_ops {
	/**
	 * Plugin instantiation
	 */
	struct cpu *(*create)(struct cpucfg const *cfg);
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
	 * Next pointer in global cpu instances linked list
	 */
	struct list_head next;
	/**
	 * Plugin descriptor
	 */
	struct cpu_desc const *cpu;
	/*
	 * Cpu unique name
	 */
	char name[CPUNAMESZ];
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
struct cpu *cpu_create(struct cpucfg const *cfg);
int cpu_destroy(struct cpu *c);
struct cpu *cpu_get(char const *name);

#endif
