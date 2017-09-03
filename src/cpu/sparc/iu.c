#include <stdlib.h>
#include <stddef.h>

#include "memory.h"
#include "utils.h"
#include "sparc.h"
#include "isn.h"

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

	scpu_set_reg(cpu, i->rd, scpu_get_pc(cpu));
	scpu_delay_jmp(cpu, rs1 + rs2);
}

static int isn_exec_jmpl(struct cpu *cpu, struct sparc_isn const *isn)
{
	int ret = 0;

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

/* ----------------- Memory instruction ------------------- */

#define be8toh(a) (a) /* Kludge */
#define htobe8(a) (a) /* Kludge */

#define ISN_EXEC_OP3_MEM_STORE(sz, c, a, b, ridx, ret) do {		\
	ret = memory_write ## sz((c)->mem, (a) + (b),			\
			htobe ## sz(scpu_get_reg(c, ridx)));		\
} while(0);

#define ISN_EXEC_OP3_MEM_STORED(sz, c, a, b, ridx, ret) do {		\
	ret = memory_write ## sz((c)->mem, (a) + (b),			\
		htobe ## sz(scpu_get_reg(c, ridx)));			\
	if(ret == 0)							\
		ret = memory_write ## sz((c)->mem,			\
				(a) + (b) + ((sz) >> 3),		\
				htobe ## sz(scpu_get_reg(c, ridx + 1)));\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADU(sz, c, a, b, ridx, ret) do {		\
	uint ## sz ##_t __r;						\
	ret = memory_read ## sz((c)->mem, (a) + (b), (void *)(&__r));	\
	scpu_set_reg(c, ridx, be ## sz ## toh(__r));			\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADS(sz, c, a, b, ridx, ret) do {		\
	uint ## sz ##_t __r;						\
	ret = memory_read ## sz((c)->mem, (a) + (b), (void *)(&__r));	\
	scpu_set_reg(c, ridx, sign_ext(be ## sz ## toh(__r), sz - 1));	\
} while(0);

#define ISN_EXEC_OP3_MEM_LOADD(sz, c, a, b, ridx, ret) do {		\
	uint ## sz ##_t __r, __r2;					\
	ret = memory_read ## sz((c)->mem, (a) + (b), (void *)(&__r));	\
	if(ret == 0) {							\
		ret = memory_read ## sz((c)->mem,			\
				(a) + (b) + ((sz) >> 3),		\
				(void *)(&__r2));			\
		scpu_set_reg(c, ridx, be ## sz ## toh(__r));		\
		scpu_set_reg(c, ridx + 1, be ## sz ## toh(__r2));	\
	}								\
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
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu, __rs1, __isn->imm,	\
				__isn->rd, ret);			\
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
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu, __rs1, __rs2,		\
				__isn->rd, ret);			\
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
static int isn_exec_ ## n(struct cpu *cpu, struct sparc_isn const *isn)	\
{									\
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);	\
	sreg pc;							\
									\
	if(!ISN_OP_ ## n(cpu)) {					\
		if(i->a)						\
			scpu_annul_delay_slot(cpu);			\
		return 0;						\
	}								\
	pc = scpu_get_pc(cpu);						\
	scpu_delay_jmp(cpu, pc + (i->disp << 2));			\
	return 0;							\
}

#define ISN_EXEC_ENTRY_Bicc(n)						\
	ISN_EXEC_ENTRY(SI_ ## n, isn_exec_ ## n)

/* 
 * Define all branches instruction callbacks that tests the proper conditional
 * code flags
 */
#define ISN_OP_BNE(c) (!scpu_get_cc_z(c))
#define ISN_OP_BE(c) (scpu_get_cc_z(c))
#define ISN_OP_BG(c)							\
	(!(scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c))))
#define ISN_OP_BLE(c)							\
	(scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c)))
#define ISN_OP_BGE(c) (!(scpu_get_cc_n(c) ^ scpu_get_cc_v(c)))
#define ISN_OP_BL(c) (scpu_get_cc_n(c) ^ scpu_get_cc_v(c))
#define ISN_OP_BGU(c) (!(scpu_get_cc_c(c) || scpu_get_cc_z(c)))
#define ISN_OP_BLEU(c) (scpu_get_cc_c(c) || scpu_get_cc_z(c))
#define ISN_OP_BCC(c) (!scpu_get_cc_c(c))
#define ISN_OP_BCS(c) (scpu_get_cc_c(c))
#define ISN_OP_BPOS(c) (!scpu_get_cc_n(c))
#define ISN_OP_BNEG(c) (scpu_get_cc_n(c))
#define ISN_OP_BVC(c) (!scpu_get_cc_v(c))
#define ISN_OP_BVS(c) (scpu_get_cc_v(c))

/* Branches instruction handler definition */
DEFINE_ISN_EXEC_Bicc(BNE);
DEFINE_ISN_EXEC_Bicc(BE);
DEFINE_ISN_EXEC_Bicc(BG);
DEFINE_ISN_EXEC_Bicc(BLE);
DEFINE_ISN_EXEC_Bicc(BGE);
DEFINE_ISN_EXEC_Bicc(BL);
DEFINE_ISN_EXEC_Bicc(BGU);
DEFINE_ISN_EXEC_Bicc(BLEU);
DEFINE_ISN_EXEC_Bicc(BCC);
DEFINE_ISN_EXEC_Bicc(BCS);
DEFINE_ISN_EXEC_Bicc(BPOS);
DEFINE_ISN_EXEC_Bicc(BNEG);
DEFINE_ISN_EXEC_Bicc(BVC);
DEFINE_ISN_EXEC_Bicc(BVS);

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
static int isn_exec_ ## n(struct cpu *cpu, struct sparc_isn const *isn)	\
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
	ISN_EXEC_Ticc(op, ISN_OP_ ## op)

#define ISN_EXEC_ENTRY_Ticc(op)						\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op)

#define ISN_OP_TA(c) (1)
#define ISN_OP_TN(c) (0)

/* Define all Ticc instructions handlers */
DEFINE_ISN_EXEC_Ticc(TA);
DEFINE_ISN_EXEC_Ticc(TN);

/* ----------- Specific register instruction ----------- */
/* Template for special register read instruction */
#define ISN_EXEC_RD_SREG(n, op)						\
static int isn_exec_rd_ ## n(struct cpu *cpu,				\
		struct sparc_isn const *i)				\
{									\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg reg;							\
									\
	if(i->fmt != SIF_OP3_REG)					\
		return -1;						\
									\
	if(scpu_get_ ## op(cpu, &reg) == 0)				\
		scpu_set_reg(cpu, __isn->rd, reg);			\
									\
	return 0;							\
}

/* Template for specific register write instruction that uses immediate */
#define ISN_EXEC_WR_SREG_IMM(c, i, o) do {				\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg __rs1;							\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	scpu_set_ ## o(c, __rs1 ^ __isn->imm);				\
} while(0)

/* Template for window register instruction that uses register */
#define ISN_EXEC_WR_SREG_REG(c, i, o) do {				\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg __rs1, __rs2;						\
									\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	scpu_set_ ## o(c, __rs1 ^ __rs2);				\
} while(0)

/* Template for special register write instruction */
#define ISN_EXEC_WR_SREG(n, op)						\
static int isn_exec_wr_ ## n(struct cpu *cpu,				\
		struct sparc_isn const *i)				\
{									\
	int ret = 0;							\
									\
	switch(i->fmt) {						\
	case SIF_OP3_IMM:						\
		ISN_EXEC_WR_SREG_IMM(cpu, i, op);			\
		break;							\
	case SIF_OP3_REG:						\
		ISN_EXEC_WR_SREG_REG(cpu, i, op);			\
		break;							\
	default:							\
		ret = -1;						\
		break;							\
	}								\
									\
	return ret;							\
}

#define DEFINE_ISN_EXEC_SREG(n, op)					\
	ISN_EXEC_WR_SREG(n, op)						\
	ISN_EXEC_RD_SREG(n, op)

#define ISN_EXEC_ENTRY_SREG(op)						\
	ISN_EXEC_ENTRY(SI_RD ## op, isn_exec_rd_ ## op),		\
	ISN_EXEC_ENTRY(SI_WR ## op, isn_exec_wr_ ## op)

/* Define all specific register instructions handlers */
DEFINE_ISN_EXEC_SREG(PSR, psr);
DEFINE_ISN_EXEC_SREG(WIM, wim);
DEFINE_ISN_EXEC_SREG(TBR, tbr);

/* -------------- Instruction execution ---------------- */

#define ISN_EXEC_ENTRY(i, f) [i] = f

/* Instruction handler dispatch array */
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
	ISN_EXEC_ENTRY(SI_SETHI, isn_exec_sethi),
	ISN_EXEC_ENTRY(SI_CALL, isn_exec_call),
	ISN_EXEC_ENTRY(SI_JMPL, isn_exec_jmpl),
	ISN_EXEC_ENTRY_LOGICAL(AND),
	ISN_EXEC_ENTRY_LOGICAL(ANDN),
	ISN_EXEC_ENTRY_LOGICAL(OR),
	ISN_EXEC_ENTRY_LOGICAL(ORN),
	ISN_EXEC_ENTRY_LOGICAL(XOR),
	ISN_EXEC_ENTRY_LOGICAL(XNOR),
	ISN_EXEC_ENTRY_ARITHMETIC(ADD),
	ISN_EXEC_ENTRY_ARITHMETIC(SUB),
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
	ISN_EXEC_ENTRY_Bicc(BNE),
	ISN_EXEC_ENTRY_Bicc(BE),
	ISN_EXEC_ENTRY_Bicc(BG),
	ISN_EXEC_ENTRY_Bicc(BLE),
	ISN_EXEC_ENTRY_Bicc(BGE),
	ISN_EXEC_ENTRY_Bicc(BL),
	ISN_EXEC_ENTRY_Bicc(BGU),
	ISN_EXEC_ENTRY_Bicc(BLEU),
	ISN_EXEC_ENTRY_Bicc(BCC),
	ISN_EXEC_ENTRY_Bicc(BCS),
	ISN_EXEC_ENTRY_Bicc(BPOS),
	ISN_EXEC_ENTRY_Bicc(BNEG),
	ISN_EXEC_ENTRY_Bicc(BVC),
	ISN_EXEC_ENTRY_Bicc(BVS),
	ISN_EXEC_ENTRY_WIN(SAVE),
	ISN_EXEC_ENTRY_WIN(RESTORE),
	ISN_EXEC_ENTRY_Ticc(TA),
	ISN_EXEC_ENTRY_Ticc(TN),
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
