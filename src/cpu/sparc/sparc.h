#ifndef _SPARC_H_
#define _SPARC_H_

#include "cpu.h"

typedef uint32_t sreg;

sreg scpu_get_reg(struct cpu *cpu, off_t ridx);
void scpu_set_reg(struct cpu *cpu, off_t ridx, sreg val);

uint8_t scpu_get_cc_n(struct cpu *cpu);
void scpu_set_cc_n(struct cpu *cpu, uint8_t val);
uint8_t scpu_get_cc_z(struct cpu *cpu);
void scpu_set_cc_z(struct cpu *cpu, uint8_t val);
uint8_t scpu_get_cc_v(struct cpu *cpu);
void scpu_set_cc_v(struct cpu *cpu, uint8_t val);
uint8_t scpu_get_cc_c(struct cpu *cpu);
void scpu_set_cc_c(struct cpu *cpu, uint8_t val);

sreg scpu_get_pc(struct cpu *cpu);
sreg scpu_get_npc(struct cpu *cpu);
void scpu_delay_jmp(struct cpu *cpu, uint32_t addr);
void scpu_annul_delay_slot(struct cpu *cpu);
void scpu_window_save(struct cpu *cpu);
void scpu_window_restore(struct cpu *cpu);

#endif