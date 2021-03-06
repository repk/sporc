#ifndef _SPARC_H_
#define _SPARC_H_

#include "cpu/cpu.h"
#include "cpu/sparc/sparc.h"
#include "dev/device.h"

typedef uint32_t sreg;
typedef uint8_t sridx;

sreg scpu_get_reg(struct cpu *cpu, sridx ridx);
void scpu_set_reg(struct cpu *cpu, sridx ridx, sreg val);

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
struct dev *scpu_get_mem(struct cpu *cpu, asi_t id);
struct dev *scpu_get_dmem(struct cpu *cpu);
struct dev *scpu_get_imem(struct cpu *cpu);
int scpu_get_psr(struct cpu *cpu, sreg *val);
int scpu_set_psr(struct cpu *cpu, sreg val);
int scpu_get_wim(struct cpu *cpu, sreg *val);
int scpu_set_wim(struct cpu *cpu, sreg val);
int scpu_get_tbr(struct cpu *cpu, sreg *val);
int scpu_set_tbr(struct cpu *cpu, sreg val);
int scpu_get_asr(struct cpu *cpu, uint8_t asr, sreg *val);
int scpu_set_asr(struct cpu *cpu, uint8_t asr, sreg v1, sreg v2);
int scpu_flush(struct cpu *cpu, addr_t addr);

void scpu_delay_jmp(struct cpu *cpu, uint32_t addr);
void scpu_annul_delay_slot(struct cpu *cpu);
void scpu_window_save(struct cpu *cpu);
void scpu_window_restore(struct cpu *cpu);
void scpu_trap(struct cpu *cpu, uint8_t tn);
void scpu_exit_trap(struct cpu *cpu, uint32_t jmp);

#endif
