#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

struct cpu_desc;

struct cpu_ops {
	struct cpu *(*create)(char const *args);
	void (*destroy)(struct cpu *cpu);
	int (*boot)(struct cpu *cpu, uintptr_t addr);
	/* Interrupt */
	int (*fetch)(struct cpu *cpu);
	int (*decode)(struct cpu *cpu);
	int (*exec)(struct cpu *cpu);
};

struct cpu_desc {
	char *name;
	struct cpu_ops const *cops;
};

struct cpu {
	struct cpu_desc const *cpu;
	struct memory *mem;
};

#define CPU_REGISTER(c)							\
	__attribute__((section(".rodata.cpu"), used))			\
	static struct cpu_desc const * const __cpu_ ## c = &c

int cpu_fetch(struct cpu *c);
int cpu_decode(struct cpu *c);
int cpu_exec(struct cpu *c);
int cpu_boot(struct cpu *c, uintptr_t addr);
struct cpu *cpu_create(char const *name, struct memory *mem, char const *args);
int cpu_destroy(struct cpu *c);

#endif
