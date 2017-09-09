#include <stdlib.h>
#include <stddef.h>

#include "memory.h"
#include "utils.h"
#include "sparc.h"
#include "isn.h"
#include "trap.h"

/* ----------------- sethi instruction Helpers ------------------- */

static int isn_exec_sethi(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_sethi *i;

	if(isn->fmt != SIF_OP2_SETHI)
		return -1;

	i = to_ifmt(op2_sethi, isn);

	scpu_set_reg(cpu, i->rd, i->imm << 10);
	return 0;
}

/* ----------------- call instruction Helper ------------------- */

static int isn_exec_call(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op1 *i;
	sreg pc;

	if(isn->fmt != SIF_OP1)
		return -1;

	i = to_ifmt(op1, isn);

	pc = scpu_get_pc(cpu);
	scpu_set_reg(cpu, 15, pc);

	scpu_delay_jmp(cpu, pc + (i->disp30 << 2));
	return 0;
}

/* ---------------- jumpl instruction Helpers ------------------ */

static inline void isn_exec_jmpl_imm(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_imm const *i = to_ifmt(op3_imm, isn);
	sreg rs1;

	rs1 = scpu_get_reg(cpu, i->rs1);

	if((rs1 + i->imm) & 0x3) {
		scpu_trap(cpu, ST_MEM_UNALIGNED);
		return;
	}

	scpu_set_reg(cpu, i->rd, scpu_get_pc(cpu));
	scpu_delay_jmp(cpu, rs1 + i->imm);
}

static inline void isn_exec_jmpl_reg(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_reg const *i = to_ifmt(op3_reg, isn);
	sreg rs1, rs2;

	rs1 = scpu_get_reg(cpu, i->rs1);
	rs2 = scpu_get_reg(cpu, i->rs2);

	if((rs1 + rs2) & 0x3) {
		scpu_trap(cpu, ST_MEM_UNALIGNED);
		return;
	}

	scpu_set_reg(cpu, i->rd, scpu_get_pc(cpu));
	scpu_delay_jmp(cpu, rs1 + rs2);
}

static int isn_exec_jmpl(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	/* TODO see JMPL ASI when RETT is on delay slot (pg 126) */
	switch(isn->fmt) {
	case SIF_OP3_IMM:
		isn_exec_jmpl_imm(cpu, isn);
		break;
	case SIF_OP3_REG:
		isn_exec_jmpl_reg(cpu, isn);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* ---------------- RETT instruction Helpers ------------------ */

static inline void isn_exec_rett_imm(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_imm const *i = to_ifmt(op3_imm, isn);
	sreg rs1;

	rs1 = scpu_get_reg(cpu, i->rs1);

	scpu_exit_trap(cpu, rs1 + i->imm);
}

static inline void isn_exec_rett_reg(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_reg const *i = to_ifmt(op3_reg, isn);
	sreg rs1, rs2;

	rs1 = scpu_get_reg(cpu, i->rs1);
	rs2 = scpu_get_reg(cpu, i->rs2);

	scpu_exit_trap(cpu, rs1 + rs2);
}

static int isn_exec_rett(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

	switch(isn->fmt) {
	case SIF_OP3_IMM:
		isn_exec_rett_imm(cpu, isn);
		break;
	case SIF_OP3_REG:
		isn_exec_rett_reg(cpu, isn);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* ----------------- ALU instruction Helpers ------------------- */

/**
 * Template for an ALU instruction that uses immediate, o is the actual
 * instruction operation (e.g. addition, substraction, ...) and cc is a
 * function that sets the proper condition codes
 */
#define ISN_EXEC_ALU_IMM(c, i, o, cc) do {				\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg __rs1, __tmp;						\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__tmp = o(__rs1, __isn->imm);					\
	cc(c, __rs1, __isn->imm, __tmp);				\
	scpu_set_reg(c, __isn->rd, __tmp);				\
} while(0)

/**
 * Template for an ALU instruction that uses register, o is the actual
 * instruction operation (e.g. addition, substraction, ...) and cc is a
 * function that sets the proper condition codes
 */
#define ISN_EXEC_ALU_REG(c, i, o, cc) do {				\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg __rs1, __rs2, __tmp;					\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	__tmp = o(__rs1, __rs2);					\
	cc(c, __rs1, __rs2, __tmp);					\
	scpu_set_reg(c, __isn->rd, __tmp);				\
} while(0)

/**
 * ALU instruction handler definition template
 */
#define ISN_EXEC_ALU(n, op, cc)						\
static int								\
isn_exec_ ## n(struct cpu *cpu, struct sparc_isn const *isn)		\
{									\
	int ret = 0;							\
									\
	switch(isn->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_ALU_IMM(cpu, isn, op, cc);			\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_ALU_REG(cpu, isn, op, cc);			\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

/* Common codition code setter callback for non conditional ALU isn */
#define ISN_ALU_CC_NOP(c, x, y, z)
/* Common codition code setter callback for all conditional logical isn */
#define ISN_ALU_CC_NZ(c, x, y, z) do {					\
	scpu_set_cc_n(c, (z >> 31) & 0x1);				\
	scpu_set_cc_z(c, ((z == 0) ? 1 : 0));				\
} while(0)

/* ----------------- Logical instruction ------------------- */

/* Define a logical instruction handler (unconditional and conditional) */
#define DEFINE_ISN_EXEC_LOGICAL(op)					\
	ISN_EXEC_ALU(op, ISN_OP_ ## op, ISN_ALU_CC_NOP)			\
	ISN_EXEC_ALU(op ## _cc, ISN_OP_ ## op, ISN_ALU_CC_NZ)

/* Define instruction handler entries for the big instruction dispatch array */
#define ISN_EXEC_ENTRY_LOGICAL(op)					\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op),			\
	ISN_EXEC_ENTRY(SI_ ## op ## CC, isn_exec_ ## op ## _cc)

/* Logical instruction operation callbacks */
#define ISN_OP_OR(a, b) ((a) | (b))
#define ISN_OP_ORN(a, b) (~((a) | (b)))
#define ISN_OP_AND(a, b) ((a) & (b))
#define ISN_OP_ANDN(a, b) (~((a) & (b)))
#define ISN_OP_XOR(a, b) ((a) ^ (b))
#define ISN_OP_XNOR(a, b) (~((a) ^ (b)))

/* Define all logical instruction handlers */
DEFINE_ISN_EXEC_LOGICAL(OR)
DEFINE_ISN_EXEC_LOGICAL(ORN)
DEFINE_ISN_EXEC_LOGICAL(AND)
DEFINE_ISN_EXEC_LOGICAL(ANDN)
DEFINE_ISN_EXEC_LOGICAL(XOR)
DEFINE_ISN_EXEC_LOGICAL(XNOR)

/* Define shift instructions */
#define DEFINE_ISN_EXEC_SHIFT(op)					\
	ISN_EXEC_ALU(op, ISN_OP_ ## op, ISN_ALU_CC_NOP)

#define ISN_EXEC_ENTRY_SHIFT(op)					\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op)

/* Shift instruction operation callbacks */
#define ISN_OP_SLL(a, b) (((uint32_t)(a)) << ((b) & 0x1f))
#define ISN_OP_SRL(a, b) (((uint32_t)(a)) >> ((b) & 0x1f))
#define ISN_OP_SRA(a, b) (((int32_t)(a)) >> ((b) & 0x1f))

/* Define all shift instruction handlers */
DEFINE_ISN_EXEC_SHIFT(SLL);
DEFINE_ISN_EXEC_SHIFT(SRL);
DEFINE_ISN_EXEC_SHIFT(SRA);

/* --------------- Arithmetic instruction ----------------- */
#define DEFINE_ISN_EXEC_ARITHMETIC(op)					\
	ISN_EXEC_ALU(op, ISN_OP_ ## op, ISN_ALU_CC_NOP)			\
	ISN_EXEC_ALU(op ## _cc, ISN_OP_ ## op, ISN_ALU_CC_ ## op)

#define ISN_EXEC_ENTRY_ARITHMETIC(op)					\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op),			\
	ISN_EXEC_ENTRY(SI_ ## op ## CC, isn_exec_ ## op ## _cc)

#define ISN_OP_ADD(a, b) ((a) + (b))
#define ISN_ALU_CC_ADD(c, x, y, z) do					\
{									\
	ISN_ALU_CC_NZ(c, x, y, z);					\
									\
	if((!((((x) >> 31) & 0x1) ^ (((y) >> 31) & 0x1))) &&		\
		((((x) >> 31) & 0x1) ^ (((z) >> 31) & 0x1)))		\
		scpu_set_cc_v(c, 1);					\
	else								\
		scpu_set_cc_v(c, 0);					\
									\
	if(((((x) >> 31) & 0x1) && ((((y) >> 31) & 0x1))) ||		\
		((!(((z) >> 31) & 0x1)) && ((((x) >> 31) & 0x1) ||	\
			(((y) >> 31) & 0x1))))				\
		scpu_set_cc_c(c, 1);					\
	else								\
		scpu_set_cc_c(c, 0);					\
} while(0)

#define ISN_OP_SUB(a, b) ((a) - (b))
#define ISN_ALU_CC_SUB(c, x, y, z) do					\
{									\
	ISN_ALU_CC_NZ(c, x, y, z);					\
									\
	if(((((x) >> 31) & 0x1) ^ (((y) >> 31) & 0x1)) &&		\
		(!((((y) >> 31) & 0x1) ^ (((z) >> 31) & 0x1))))		\
		scpu_set_cc_v(c, 1);					\
	else								\
		scpu_set_cc_v(c, 0);					\
									\
	if(((!(((x) >> 31) & 0x1)) && (((y) >> 31) & 0x1)) ||		\
		((((y) >> 31) & 0x1) && ((!(((x) >> 31) & 0x1)) ||	\
			(((y) >> 31) & 0x1))))				\
		scpu_set_cc_c(c, 1);					\
	else								\
		scpu_set_cc_c(c, 0);					\
} while(0)

DEFINE_ISN_EXEC_ARITHMETIC(ADD)
DEFINE_ISN_EXEC_ARITHMETIC(SUB)

/* ---------------- Multiply instruction ------------------ */

#define ISN_EXEC_MUL_REG(c, isn, op) do					\
{									\
	struct sparc_ifmt_op3_reg const *__i = to_ifmt(op3_reg, isn);	\
	sreg __rs1, __rs2, __rd;					\
									\
	__rs1 = scpu_get_reg(c, __i->rs1);				\
	__rs2 = scpu_get_reg(c, __i->rs2);				\
	ISN_OP_ ## op(c, __rd, __rs1, __rs2);				\
	scpu_set_reg(c, __i->rd, __rd);					\
} while(0)

#define ISN_EXEC_MUL_IMM(c, isn, op) do					\
{									\
	struct sparc_ifmt_op3_imm const *__i = to_ifmt(op3_imm, isn);	\
	sreg __rd, __rs1;						\
									\
	__rs1 = scpu_get_reg(c, __i->rs1);				\
	ISN_OP_ ## op(c, __rd, __rs1, __i->imm);			\
	scpu_set_reg(c, __i->rd, __rd);					\
} while(0)

#define ISN_EXEC_MUL(op)						\
static int isn_exec_ ## op(struct cpu *c, struct sparc_isn const *i)	\
{									\
	int ret = 0;							\
									\
	switch((i)->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_MUL_IMM(c, i, op);				\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_MUL_REG(c, i, op);				\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}
#define ISN_EXEC_ENTRY_MUL(op)						\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op)

#define ISN_OP_MULSCC(c, rd, rs1, rsi) do				\
{									\
	sreg __y;							\
	uint32_t __tmp = (((uint32_t)(rs1)) >> 1) |			\
		((scpu_get_cc_v(c) ^ scpu_get_cc_n(c)) << 31);		\
									\
	/* Step add */							\
	scpu_get_asr(c, 0, &__y);					\
	if(__y & 0x1) {							\
		__rd = ISN_OP_ADD(__tmp, rsi);				\
		ISN_ALU_CC_ADD(c, __tmp, rsi, rd);			\
	} else {							\
		rd = ISN_OP_ADD(__tmp, 0);				\
		ISN_ALU_CC_ADD(c, __tmp, 0, rd);			\
	}								\
									\
	/* Update y register */						\
	__y = (((uint32_t)(__y)) >> 1) |				\
		(((uint32_t)((rs1) & 0x1)) << 31);			\
	scpu_set_asr(c, 0, __y, 0);					\
} while(0)

#define ISN_OP_UMUL(c, rd, rs1, rsi) do					\
{									\
	uint64_t __res = ((uint64_t)((uint32_t)(rs1))) *		\
			((uint32_t)(rsi));				\
	rd = __res & 0xffffffff;					\
	scpu_set_asr(c, 0, __res >> 32, 0);				\
} while(0)

#define ISN_OP_UMULCC(c, rd, rs1, rsi) do				\
{									\
	ISN_OP_UMUL(c, rd, rs1, rsi);					\
	ISN_ALU_CC_NZ(c, rs1, rsi, rd);					\
	scpu_set_cc_v(c, 0);						\
	scpu_set_cc_c(c, 0);						\
} while(0)

#define ISN_OP_SMUL(c, rd, rs1, rsi) do					\
{									\
	int64_t __res = ((int64_t)((int32_t)(rs1))) *			\
			((int32_t)(rsi));				\
	rd = __res & 0xffffffff;					\
	scpu_set_asr(c, 0, __res >> 32, 0);				\
} while(0)

#define ISN_OP_SMULCC(c, rd, rs1, rsi) do				\
{									\
	ISN_OP_SMUL(c, rd, rs1, rsi);					\
	ISN_ALU_CC_NZ(c, rs1, rsi, rd);					\
	scpu_set_cc_v(c, 0);						\
	scpu_set_cc_c(c, 0);						\
} while(0)

/* Define a multiply / divide instruction */
ISN_EXEC_MUL(MULSCC);
ISN_EXEC_MUL(UMUL);
ISN_EXEC_MUL(UMULCC);
ISN_EXEC_MUL(SMUL);
ISN_EXEC_MUL(SMULCC);

/* ----------------- Memory instruction ------------------- */

#define be8toh(a) (a) /* Kludge */
#define htobe8(a) (a) /* Kludge */

/* Check address is aligned on a power of two bit size */
#define MEM_ALIGN(a, s) (((a) & (((s) >> 3) - 1)) == 0)

#define ISN_EXEC_OP3_MEM_STORE(sz, c, a, ridx) do {			\
	int __ret;							\
	__ret = memory_write ## sz((c)->mem, a,				\
			htobe ## sz(scpu_get_reg(c, ridx)));		\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
} while(0);

#define ISN_EXEC_OP3_MEM_STORED(sz, c, a, ridx) do {			\
	int __ret;							\
	/* rd should be even */						\
	if((ridx) & 0x1) {						\
		scpu_trap(c, ST_ILL_ISN);				\
		break;							\
	}								\
	__ret = memory_write ## sz((c)->mem, a,				\
			htobe ## sz(scpu_get_reg(c, ridx)));		\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
	__ret = memory_write ## sz((c)->mem, (a) + ((sz) >> 3),		\
			htobe ## sz(scpu_get_reg(c, ridx + 1)));	\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADU(sz, c, a, ridx) do {			\
	int __ret;							\
	uint ## sz ##_t __r;						\
	__ret = memory_read ## sz((c)->mem, a, (void *)(&__r));		\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
	scpu_set_reg(c, ridx, be ## sz ## toh(__r));			\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADS(sz, c, a, ridx) do {			\
	int __ret;							\
	uint ## sz ##_t __r;						\
	__ret = memory_read ## sz((c)->mem, a, (void *)(&__r));		\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
	scpu_set_reg(c, ridx, sign_ext(be ## sz ## toh(__r), sz - 1));	\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADD(sz, c, a, ridx) do {			\
	int __ret;							\
	uint ## sz ##_t __r, __r2;					\
	/* rd should be even */						\
	if((ridx) & 0x1) {						\
		scpu_trap(c, ST_ILL_ISN);				\
		break;							\
	}								\
	__ret = memory_read ## sz((c)->mem, a, (void *)(&__r));		\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
	__ret = memory_read ## sz((c)->mem, (a) + ((sz) >> 3),		\
			(void *)(&__r2));				\
	if(__ret != 0) {						\
		scpu_trap(c, ST_DACCESS_EXCEP);				\
		break;							\
	}								\
	scpu_set_reg(c, ridx, be ## sz ## toh(__r));			\
	scpu_set_reg(c, ridx + 1, be ## sz ## toh(__r2));		\
} while(0);

/**
 * Template for defining an handler for an memory (fetch/load) instruction,
 * op is the actual memory operation callback (load, fetch, ...) and sz is
 * the size of the memory to load or fetch.
 */
#define DEFINE_ISN_EXEC_MEM(n, op, sz)					\
static int								\
isn_exec_ ## n(struct cpu *cpu, struct sparc_isn const *isn)		\
{									\
	int ret = 0;							\
									\
	switch(isn->fmt) {						\
	case SIF_OP3_IMM:						\
	{								\
		struct sparc_ifmt_op3_imm const *__isn =		\
				to_ifmt(op3_imm, isn);			\
		sreg __rs1;						\
									\
		__rs1 = scpu_get_reg(cpu, __isn->rs1);			\
		if(!MEM_ALIGN(__rs1 + __isn->imm, sz)) {		\
			scpu_trap(cpu, ST_MEM_UNALIGNED);		\
			break;						\
		}							\
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu, __rs1 + __isn->imm,	\
				__isn->rd);				\
		break;							\
	}								\
	case SIF_OP3_REG:						\
	{								\
		struct sparc_ifmt_op3_reg const *__isn =		\
				to_ifmt(op3_reg, isn);			\
		sreg __rs1, __rs2;					\
									\
		__rs1 = scpu_get_reg(cpu, __isn->rs1);			\
		__rs2 = scpu_get_reg(cpu, __isn->rs2);			\
		if(!MEM_ALIGN(__rs1 + __rs2, sz)) {			\
			scpu_trap(cpu, ST_MEM_UNALIGNED);		\
			break;						\
		}							\
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu, __rs1 + __rs2,		\
				__isn->rd);				\
		break;							\
	}								\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

#define ISN_EXEC_ENTRY_MEM(n)					\
	ISN_EXEC_ENTRY(SI_ ## n, isn_exec_ ## n)

/* Define all type of memory instructions */
DEFINE_ISN_EXEC_MEM(STB, STORE, 8)
DEFINE_ISN_EXEC_MEM(STH, STORE, 16)
DEFINE_ISN_EXEC_MEM(ST, STORE, 32)
DEFINE_ISN_EXEC_MEM(STD, STORED, 32) /* Double memory_write32 */
DEFINE_ISN_EXEC_MEM(LDSB, LOADS, 8)
DEFINE_ISN_EXEC_MEM(LDUB, LOADU, 8)
DEFINE_ISN_EXEC_MEM(LDSH, LOADS, 16)
DEFINE_ISN_EXEC_MEM(LDUH, LOADU, 16)
DEFINE_ISN_EXEC_MEM(LD, LOADU, 32)
DEFINE_ISN_EXEC_MEM(LDD, LOADD, 32) /* Double memory_read32 */

/* --------- Condition code flags test macros ---------- */

#define ISN_ICC_OP_A(c) (1)
#define ISN_ICC_OP_N(c) (0)
#define ISN_ICC_OP_NE(c) (!scpu_get_cc_z(c))
#define ISN_ICC_OP_E(c) (scpu_get_cc_z(c))
#define ISN_ICC_OP_G(c)							\
	(!(scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c))))
#define ISN_ICC_OP_LE(c)							\
	(scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c)))
#define ISN_ICC_OP_GE(c) (!(scpu_get_cc_n(c) ^ scpu_get_cc_v(c)))
#define ISN_ICC_OP_L(c) (scpu_get_cc_n(c) ^ scpu_get_cc_v(c))
#define ISN_ICC_OP_GU(c) (!(scpu_get_cc_c(c) || scpu_get_cc_z(c)))
#define ISN_ICC_OP_LEU(c) (scpu_get_cc_c(c) || scpu_get_cc_z(c))
#define ISN_ICC_OP_CC(c) (!scpu_get_cc_c(c))
#define ISN_ICC_OP_CS(c) (scpu_get_cc_c(c))
#define ISN_ICC_OP_POS(c) (!scpu_get_cc_n(c))
#define ISN_ICC_OP_NEG(c) (scpu_get_cc_n(c))
#define ISN_ICC_OP_VC(c) (!scpu_get_cc_v(c))
#define ISN_ICC_OP_VS(c) (scpu_get_cc_v(c))

/* --------------- Bicc instructions ------------------- */

static int isn_exec_bn(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);

	if(i->a)
		scpu_annul_delay_slot(cpu);
	return 0;
}

static int isn_exec_ba(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);
	sreg pc;

	pc = scpu_get_pc(cpu);
	scpu_delay_jmp(cpu, pc + (i->disp << 2));

	if(i->a)
		scpu_annul_delay_slot(cpu);
	return 0;
}

/* Define a branch instruction handler template */
#define DEFINE_ISN_EXEC_Bicc(n)						\
static int								\
isn_exec_B ## n(struct cpu *cpu, struct sparc_isn const *isn)		\
{									\
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);	\
	sreg pc;							\
									\
	if(!ISN_ICC_OP_ ## n(cpu)) {					\
		if(i->a)						\
			scpu_annul_delay_slot(cpu);			\
		return 0;						\
	}								\
	pc = scpu_get_pc(cpu);						\
	scpu_delay_jmp(cpu, pc + (i->disp << 2));			\
	return 0;							\
}

#define ISN_EXEC_ENTRY_Bicc(n)						\
	ISN_EXEC_ENTRY(SI_B ## n, isn_exec_B ## n)

/* Branches instruction handler definition */
DEFINE_ISN_EXEC_Bicc(NE);
DEFINE_ISN_EXEC_Bicc(E);
DEFINE_ISN_EXEC_Bicc(G);
DEFINE_ISN_EXEC_Bicc(LE);
DEFINE_ISN_EXEC_Bicc(GE);
DEFINE_ISN_EXEC_Bicc(L);
DEFINE_ISN_EXEC_Bicc(GU);
DEFINE_ISN_EXEC_Bicc(LEU);
DEFINE_ISN_EXEC_Bicc(CC);
DEFINE_ISN_EXEC_Bicc(CS);
DEFINE_ISN_EXEC_Bicc(POS);
DEFINE_ISN_EXEC_Bicc(NEG);
DEFINE_ISN_EXEC_Bicc(VC);
DEFINE_ISN_EXEC_Bicc(VS);

/* ---------- Window isn (save/restore) execution ------------ */

/* Template for window register instruction that uses immediate */
#define ISN_EXEC_WIN_IMM(c, i, o) do {					\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg __rs1;							\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	o(c);								\
	scpu_set_reg(c, __isn->rd, __rs1 + __isn->imm);			\
} while(0)

/* Template for window register instruction that uses register */
#define ISN_EXEC_WIN_REG(c, i, o) do {					\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg __rs1, __rs2;						\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	o(c);								\
	scpu_set_reg(c, __isn->rd, __rs1 + __rs2);			\
} while(0)

/* Template for window register instruction */
#define ISN_EXEC_WIN(n, op)						\
static int isn_exec_ ## n(struct cpu *cpu, struct sparc_isn const *isn)	\
{									\
	int ret = 0;							\
									\
	switch(isn->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_WIN_IMM(cpu, isn, op);				\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_WIN_REG(cpu, isn, op);				\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

#define DEFINE_ISN_EXEC_WIN(op)						\
	ISN_EXEC_WIN(op, ISN_OP_ ## op)

#define ISN_EXEC_ENTRY_WIN(op)						\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op)

#define ISN_OP_SAVE scpu_window_save
#define ISN_OP_RESTORE scpu_window_restore

/* Define all window register instruction handlers */
DEFINE_ISN_EXEC_WIN(SAVE);
DEFINE_ISN_EXEC_WIN(RESTORE);

/* ---------------- Trap isn execution ------------------*/
/* Template for trap instruction that uses immediate */
#define ISN_EXEC_Ticc_IMM(c, i, o) do {					\
	struct sparc_ifmt_op3_icc_imm const *__isn =			\
		to_ifmt(op3_icc_imm, i);				\
	sreg __rs1;							\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	scpu_trap(c, 128 + ((__rs1 + __isn->imm)  & (0x7f)));		\
} while(0)

/* Template for trap instruction that uses register */
#define ISN_EXEC_Ticc_REG(c, i, o) do {					\
	struct sparc_ifmt_op3_icc_reg const *__isn =			\
		to_ifmt(op3_icc_reg, i);				\
	sreg __rs1, __rs2;						\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	scpu_trap(c, 128 + ((__rs1 + __rs2) & (0x7f)));			\
} while(0)

/* Template for trap instruction */
#define ISN_EXEC_Ticc(n, op)						\
static int								\
isn_exec_T ## n(struct cpu *cpu, struct sparc_isn const *isn)		\
{									\
	int ret = 0;							\
									\
	if(!op(cpu))							\
		return ret;						\
									\
	switch(isn->fmt) {						\
	case SIF_OP3_ICC_IMM:						\
		ISN_EXEC_Ticc_IMM(cpu, isn, op);			\
		break;							\
	case SIF_OP3_ICC_REG:						\
		ISN_EXEC_Ticc_REG(cpu, isn, op);			\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

#define DEFINE_ISN_EXEC_Ticc(op)					\
	ISN_EXEC_Ticc(op, ISN_ICC_OP_ ## op)

#define ISN_EXEC_ENTRY_Ticc(op)						\
	ISN_EXEC_ENTRY(SI_T ## op, isn_exec_T ## op)

/* Define all Ticc instructions handlers */
DEFINE_ISN_EXEC_Ticc(A);
DEFINE_ISN_EXEC_Ticc(N);
DEFINE_ISN_EXEC_Ticc(NE);
DEFINE_ISN_EXEC_Ticc(E);
DEFINE_ISN_EXEC_Ticc(G);
DEFINE_ISN_EXEC_Ticc(LE);
DEFINE_ISN_EXEC_Ticc(GE);
DEFINE_ISN_EXEC_Ticc(L);
DEFINE_ISN_EXEC_Ticc(GU);
DEFINE_ISN_EXEC_Ticc(LEU);
DEFINE_ISN_EXEC_Ticc(CC);
DEFINE_ISN_EXEC_Ticc(CS);
DEFINE_ISN_EXEC_Ticc(POS);
DEFINE_ISN_EXEC_Ticc(NEG);
DEFINE_ISN_EXEC_Ticc(VC);
DEFINE_ISN_EXEC_Ticc(VS);

/* ----------- Specific register instruction ----------- */

/* Template for special register read instruction */
#define ISN_EXEC_RD_SREG(n, f, op)					\
static int isn_exec_rd_ ## n(struct cpu *cpu,				\
		struct sparc_isn const *i)				\
{									\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg reg;							\
									\
	if(i->fmt != SIF_OP3_REG)					\
		return -1;						\
									\
	if(f(op, cpu, __isn->rs1, &reg) == 0)				\
		scpu_set_reg(cpu, __isn->rd, reg);			\
									\
	return 0;							\
}

/* Template for specific register write instruction that uses immediate */
#define ISN_EXEC_WR_SREG_IMM(c, i, f, o) do {				\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg __rs1;							\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	f(o, c, __isn->rd, __rs1, __isn->imm);				\
} while(0)

/* Template for window register instruction that uses register */
#define ISN_EXEC_WR_SREG_REG(c, i, f, o) do {				\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg __rs1, __rs2;						\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	f(o, c, __isn->rd, __rs1, __rs2);				\
} while(0)

/* Template for special register write instruction */
#define ISN_EXEC_WR_SREG(n, f, op)					\
static int isn_exec_wr_ ## n(struct cpu *cpu,				\
		struct sparc_isn const *i)				\
{									\
	int ret = 0;							\
									\
	switch(i->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_WR_SREG_IMM(cpu, i, f, op);			\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_WR_SREG_REG(cpu, i, f, op);			\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

/* Format correctly argument for special register setter/getter */
#define SREG_SIMPLE_SET(op, c, rd, v1, v2) scpu_set_ ## op(c, v1 ^ v2)
#define SREG_ASR_SET(op, c, rd, v1, v2)					\
	scpu_set_ ## op(c, rd, v1, v2)
#define SREG_SIMPLE_GET(op, c, rs1, res) scpu_get_ ## op(c, res)
#define SREG_ASR_GET(op, c, rs1, res)					\
	scpu_get_ ## op(c, rs1, res)

#define DEFINE_ISN_EXEC_SIMPLE_SREG(n, op)				\
	ISN_EXEC_WR_SREG(n, SREG_SIMPLE_SET, op)			\
	ISN_EXEC_RD_SREG(n, SREG_SIMPLE_GET, op)

#define DEFINE_ISN_EXEC_ASR_SREG(n, op)					\
	ISN_EXEC_WR_SREG(n, SREG_ASR_SET, op)				\
	ISN_EXEC_RD_SREG(n, SREG_ASR_GET, op)

#define ISN_EXEC_ENTRY_SREG(op)						\
	ISN_EXEC_ENTRY(SI_RD ## op, isn_exec_rd_ ## op),		\
	ISN_EXEC_ENTRY(SI_WR ## op, isn_exec_wr_ ## op)

/* Define all specific register instructions handlers */
DEFINE_ISN_EXEC_ASR_SREG(ASR, asr);
DEFINE_ISN_EXEC_SIMPLE_SREG(PSR, psr);
DEFINE_ISN_EXEC_SIMPLE_SREG(WIM, wim);
DEFINE_ISN_EXEC_SIMPLE_SREG(TBR, tbr);

/* -------------- Instruction execution ---------------- */

#define ISN_EXEC_ENTRY(i, f) [i] = f

/* Instruction handler dispatch array */
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
	ISN_EXEC_ENTRY(SI_SETHI, isn_exec_sethi),
	ISN_EXEC_ENTRY(SI_CALL, isn_exec_call),
	ISN_EXEC_ENTRY(SI_JMPL, isn_exec_jmpl),
	ISN_EXEC_ENTRY(SI_RETT, isn_exec_rett),
	ISN_EXEC_ENTRY_LOGICAL(AND),
	ISN_EXEC_ENTRY_LOGICAL(ANDN),
	ISN_EXEC_ENTRY_LOGICAL(OR),
	ISN_EXEC_ENTRY_LOGICAL(ORN),
	ISN_EXEC_ENTRY_LOGICAL(XOR),
	ISN_EXEC_ENTRY_LOGICAL(XNOR),
	ISN_EXEC_ENTRY_SHIFT(SLL),
	ISN_EXEC_ENTRY_SHIFT(SRL),
	ISN_EXEC_ENTRY_SHIFT(SRA),
	ISN_EXEC_ENTRY_ARITHMETIC(ADD),
	ISN_EXEC_ENTRY_ARITHMETIC(SUB),
	ISN_EXEC_ENTRY_MUL(MULSCC),
	ISN_EXEC_ENTRY_MUL(UMUL),
	ISN_EXEC_ENTRY_MUL(UMULCC),
	ISN_EXEC_ENTRY_MUL(SMUL),
	ISN_EXEC_ENTRY_MUL(SMULCC),
	ISN_EXEC_ENTRY_MEM(LDSB),
	ISN_EXEC_ENTRY_MEM(LDSH),
	ISN_EXEC_ENTRY_MEM(LDUB),
	ISN_EXEC_ENTRY_MEM(LDUH),
	ISN_EXEC_ENTRY_MEM(LD),
	ISN_EXEC_ENTRY_MEM(LDD),
	ISN_EXEC_ENTRY_MEM(STB),
	ISN_EXEC_ENTRY_MEM(STH),
	ISN_EXEC_ENTRY_MEM(ST),
	ISN_EXEC_ENTRY_MEM(STD),
	ISN_EXEC_ENTRY(SI_BN, isn_exec_bn),
	ISN_EXEC_ENTRY(SI_BA, isn_exec_ba),
	ISN_EXEC_ENTRY_Bicc(NE),
	ISN_EXEC_ENTRY_Bicc(E),
	ISN_EXEC_ENTRY_Bicc(G),
	ISN_EXEC_ENTRY_Bicc(LE),
	ISN_EXEC_ENTRY_Bicc(GE),
	ISN_EXEC_ENTRY_Bicc(L),
	ISN_EXEC_ENTRY_Bicc(GU),
	ISN_EXEC_ENTRY_Bicc(LEU),
	ISN_EXEC_ENTRY_Bicc(CC),
	ISN_EXEC_ENTRY_Bicc(CS),
	ISN_EXEC_ENTRY_Bicc(POS),
	ISN_EXEC_ENTRY_Bicc(NEG),
	ISN_EXEC_ENTRY_Bicc(VC),
	ISN_EXEC_ENTRY_Bicc(VS),
	ISN_EXEC_ENTRY_WIN(SAVE),
	ISN_EXEC_ENTRY_WIN(RESTORE),
	ISN_EXEC_ENTRY_Ticc(A),
	ISN_EXEC_ENTRY_Ticc(N),
	ISN_EXEC_ENTRY_Ticc(NE),
	ISN_EXEC_ENTRY_Ticc(E),
	ISN_EXEC_ENTRY_Ticc(G),
	ISN_EXEC_ENTRY_Ticc(LE),
	ISN_EXEC_ENTRY_Ticc(GE),
	ISN_EXEC_ENTRY_Ticc(L),
	ISN_EXEC_ENTRY_Ticc(GU),
	ISN_EXEC_ENTRY_Ticc(LEU),
	ISN_EXEC_ENTRY_Ticc(CC),
	ISN_EXEC_ENTRY_Ticc(CS),
	ISN_EXEC_ENTRY_Ticc(POS),
	ISN_EXEC_ENTRY_Ticc(NEG),
	ISN_EXEC_ENTRY_Ticc(VC),
	ISN_EXEC_ENTRY_Ticc(VS),
	ISN_EXEC_ENTRY_SREG(ASR),
	ISN_EXEC_ENTRY_SREG(PSR),
	ISN_EXEC_ENTRY_SREG(WIM),
	ISN_EXEC_ENTRY_SREG(TBR),
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
