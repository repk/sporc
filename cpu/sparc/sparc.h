#ifndef _SPARC_H_
#define _SPARC_H_

#include "cpu.h"

typedef uint32_t sreg;

sreg *scpu_get_reg(struct cpu *cpu, off_t ridx);

#endif
