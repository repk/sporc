#include <stdlib.h>
#include <stddef.h>

#include "memory.h"
#include "utils.h"
#include "sparc.h"
#include "isn.h"

static int isn_exec_sethi(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_sethi *i;
	sreg *rd;

	if(isn->fmt != SIF_OP2_SETHI)
		return -1;

	i = to_ifmt(op2_sethi, isn);

	rd = scpu_get_reg(cpu, i->rd);
	*rd = i->imm << 10;
	return 0;
}

#define ISN_EXEC_ALU_IMM(c, i, o) do {					\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg *__rd, *__rs1;						\
									\
	__rd = scpu_get_reg(c, __isn->rd);				\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	*__rd = o(*__rs1, __isn->imm);					\
} while(0)

#define ISN_EXEC_ALU_REG(c, i, o) do {					\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg *__rd, *__rs1, *__rs2;					\
									\
	__rd = scpu_get_reg(c, __isn->rd);				\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	*__rd = o(*__rs1, *__rs2);					\
} while(0)

#define ISN_EXEC_ALU(c, i, op, ret) do {				\
	switch((i)->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_ALU_IMM(c, i, op);				\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_ALU_REG(c, i, op);				\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
} while(0)

#define ISN_OP_OR(a, b) ((a) | (b))
static int isn_exec_or(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_ALU(cpu, isn, ISN_OP_OR, ret);

	return ret;
}

#define ISN_EXEC_ENTRY(i, f) [i] = f
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
	ISN_EXEC_ENTRY(SI_SETHI, isn_exec_sethi),
	ISN_EXEC_ENTRY(SI_OR, isn_exec_or),
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
