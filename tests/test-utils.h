#ifndef _TEST_UTILS_H_
#define _TEST_UTILS_H_

#include <cpu.h>

uint32_t test_cpu_get_reg(struct cpu *cpu, off_t ridx);
uint32_t test_cpu_get_mem32(struct cpu *cpu, uintptr_t addr);
uint16_t test_cpu_get_mem16(struct cpu *cpu, uintptr_t addr);
uint8_t test_cpu_get_mem8(struct cpu *cpu, uintptr_t addr);
int test_cpu_step(struct cpu *cpu);
struct cpu *test_cpu_open(int argc, char **argv, char const *memfile,
		size_t memsz);
void test_cpu_close(struct cpu *cpu);

#endif
