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

#define ISN_OP_AND(a, b) ((a) & (b))
static int isn_exec_and(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_ALU(cpu, isn, ISN_OP_AND, ret);

	return ret;
}

#define be8toh(a) (a) /* Kludge */
#define htobe8(a) (a) /* Kludge */

#define ISN_EXEC_OP3_MEM_STORE(sz, m, a, b, c, ret) do {		\
	ret = memory_write ## sz(m, *(a) + *(b), htobe ## sz(*(c)));	\
} while(0);

#define ISN_EXEC_OP3_MEM_STORED(sz, m, a, b, c, ret) do {		\
	ret = memory_write ## sz(m, *(a) + *(b), htobe ## sz(*(c)));	\
	if(ret == 0)							\
		ret = memory_write ## sz((m),				\
				*(a) + *(b) + ((sz) >> 3),		\
				htobe ## sz(*(c + 1)));			\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADU(sz, m, a, b, c, ret) do {		\
	ret = memory_read ## sz(m, *(a) + *(b), (void *)(c));		\
	*(c) = be ## sz ## toh(*(c));					\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADS(sz, m, a, b, c, ret) do {		\
	ret = memory_read ## sz(m, *(a) + *(b), (void *)(c));		\
	*(c) = sign_ext(be ## sz ## toh(*(c)), sz - 1);			\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADD(sz, m, a, b, c, ret) do {		\
	ret = memory_read ## sz(m, *(a) + *(b), (void *)(c));		\
	if(ret == 0) {							\
		ret = memory_read ## sz((m), *(a) + *(b) + ((sz) >> 3),	\
				(void *)(c + 1));			\
		*(c) = be ## sz ## toh(*(c));				\
		*(c + 1) = be ## sz ## toh(*(c + 1));			\
	}								\
} while(0);

#define ISN_EXEC_OP3_MEM(c, i, op, sz, ret) do {			\
	switch((i)->fmt) {						\
	case SIF_OP3_IMM:						\
	{								\
		struct sparc_ifmt_op3_imm const *__isn =		\
				to_ifmt(op3_imm, i);			\
		sreg *__rd, *__rs1;					\
									\
		__rd = scpu_get_reg(c, __isn->rd);			\
		__rs1 = scpu_get_reg(c, __isn->rs1);			\
		ISN_EXEC_OP3_MEM_ ## op(sz, (c)->mem, __rs1,		\
				&__isn->imm, __rd, ret);		\
		break;							\
	}								\
	case SIF_OP3_REG:						\
	{								\
		struct sparc_ifmt_op3_reg const *__isn =		\
				to_ifmt(op3_reg, i);			\
		sreg *__rd, *__rs1, *__rs2;				\
									\
		__rd = scpu_get_reg(c, __isn->rd);			\
		__rs1 = scpu_get_reg(c, __isn->rs1);			\
		__rs2 = scpu_get_reg(c, __isn->rs2);			\
		ISN_EXEC_OP3_MEM_ ## op(sz, (c)->mem, __rs1, __rs2,	\
				__rd, ret);				\
		break;							\
	}								\
	default:							\
		ret = -1;						\
		break;							\
	}								\
} while(0)

static int isn_exec_stb(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, STORE, 8, ret);
	return ret;
}

static int isn_exec_sth(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, STORE, 16, ret);
	return ret;
}

static int isn_exec_st(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, STORE, 32, ret);
	return ret;
}

static int isn_exec_std(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	/* Double memory_write32() */
	ISN_EXEC_OP3_MEM(cpu, isn, STORED, 32, ret);
	return ret;
}

static int isn_exec_ldsb(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, LOADS, 8, ret);
	return ret;
}

static int isn_exec_ldsh(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, LOADS, 16, ret);
	return ret;
}

static int isn_exec_ldub(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, LOADU, 8, ret);
	return ret;
}

static int isn_exec_lduh(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, LOADU, 16, ret);
	return ret;
}

static int isn_exec_ld(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	ISN_EXEC_OP3_MEM(cpu, isn, LOADU, 32, ret);
	return ret;
}

static int isn_exec_ldd(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	/* Double memory_read32 */
	ISN_EXEC_OP3_MEM(cpu, isn, LOADD, 32, ret);
	return ret;
}

#define ISN_EXEC_ENTRY(i, f) [i] = f
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
	ISN_EXEC_ENTRY(SI_SETHI, isn_exec_sethi),
	ISN_EXEC_ENTRY(SI_AND, isn_exec_and),
	ISN_EXEC_ENTRY(SI_OR, isn_exec_or),
	ISN_EXEC_ENTRY(SI_LDSB, isn_exec_ldsb),
	ISN_EXEC_ENTRY(SI_LDSH, isn_exec_ldsh),
	ISN_EXEC_ENTRY(SI_LDUB, isn_exec_ldub),
	ISN_EXEC_ENTRY(SI_LDUH, isn_exec_lduh),
	ISN_EXEC_ENTRY(SI_LD, isn_exec_ld),
	ISN_EXEC_ENTRY(SI_LDD, isn_exec_ldd),
	ISN_EXEC_ENTRY(SI_STB, isn_exec_stb),
	ISN_EXEC_ENTRY(SI_STH, isn_exec_sth),
	ISN_EXEC_ENTRY(SI_ST, isn_exec_st),
	ISN_EXEC_ENTRY(SI_STD, isn_exec_std),
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
