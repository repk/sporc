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
	sreg *rd;

	if(isn->fmt != SIF_OP2_SETHI)
		return -1;

	i = to_ifmt(op2_sethi, isn);

	rd = scpu_get_reg(cpu, i->rd);
	*rd = i->imm << 10;
	return 0;
}

/* ----------------- call instruction Helper ------------------- */

static int isn_exec_call(struct cpu *cpu, struct sparc_isn const *isn)
{
	struct sparc_ifmt_op1 *i;
	sreg pc, *o7;

	if(isn->fmt != SIF_OP1)
		return -1;

	i = to_ifmt(op1, isn);

	pc = scpu_get_pc(cpu);
	o7 = scpu_get_reg(cpu, 15);
	*o7 = pc;

	scpu_delay_jmp(cpu, pc + (i->disp30 << 2));
	return 0;
}

/* ---------------- jumpl instruction Helpers ------------------ */

static inline void isn_exec_jmpl_imm(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_imm const *i = to_ifmt(op3_imm, isn);
	sreg *rd, *rs1;

	rd = scpu_get_reg(cpu, i->rd);
	rs1 = scpu_get_reg(cpu, i->rs1);

	*rd = scpu_get_pc(cpu);
	scpu_delay_jmp(cpu, *rs1 + i->imm);
}

static inline void isn_exec_jmpl_reg(struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_reg const *i = to_ifmt(op3_reg, isn);
	sreg *rd, *rs1, *rs2;

	rd = scpu_get_reg(cpu, i->rd);
	rs1 = scpu_get_reg(cpu, i->rs1);
	rs2 = scpu_get_reg(cpu, i->rs2);

	*rd = scpu_get_pc(cpu);
	scpu_delay_jmp(cpu, *rs1 + *rs2);
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

#define ISN_EXEC_ALU_IMM(c, i, o, cc) do {				\
	struct sparc_ifmt_op3_imm const *__isn = to_ifmt(op3_imm, i);	\
	sreg *__rd, *__rs1, __tmp;					\
									\
	__rd = scpu_get_reg(c, __isn->rd);				\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__tmp = o(*__rs1, __isn->imm);					\
	cc(c, *__rs1, __isn->imm, __tmp);				\
	*__rd = __tmp;							\
} while(0)

#define ISN_EXEC_ALU_REG(c, i, o, cc) do {				\
	struct sparc_ifmt_op3_reg const *__isn = to_ifmt(op3_reg, i);	\
	sreg *__rd, *__rs1, *__rs2, __tmp;				\
									\
	__rd = scpu_get_reg(c, __isn->rd);				\
	__rs1 = scpu_get_reg(c, __isn->rs1);				\
	__rs2 = scpu_get_reg(c, __isn->rs2);				\
	__tmp = o(*__rs1, *__rs2);					\
	cc(c, *__rs1, *__rs2, __tmp);					\
	*__rd = __tmp;							\
} while(0)

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

#define ISN_ALU_CC_NOP(c, x, y, z)
#define ISN_ALU_CC_NZ(c, x, y, z) do {					\
	scpu_set_cc_n(c, (z >> 31) & 0x1);				\
	scpu_set_cc_z(c, ((z == 0) ? 1 : 0));				\
} while(0)

/* ----------------- Logical instruction ------------------- */

#define DEFINE_ISN_EXEC_LOGICAL(op)					\
	ISN_EXEC_ALU(op, ISN_OP_ ## op, ISN_ALU_CC_NOP)			\
	ISN_EXEC_ALU(op ## _cc, ISN_OP_ ## op, ISN_ALU_CC_NZ)

#define ISN_EXEC_ENTRY_LOGICAL(op)					\
	ISN_EXEC_ENTRY(SI_ ## op, isn_exec_ ## op),			\
	ISN_EXEC_ENTRY(SI_ ## op ## CC, isn_exec_ ## op ## _cc)

#define ISN_OP_OR(a, b) ((a) | (b))
#define ISN_OP_ORN(a, b) (~((a) | (b)))
#define ISN_OP_AND(a, b) ((a) & (b))
#define ISN_OP_ANDN(a, b) (~((a) & (b)))
#define ISN_OP_XOR(a, b) ((a) ^ (b))
#define ISN_OP_XNOR(a, b) (~((a) ^ (b)))

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
		sreg *__rd, *__rs1;					\
									\
		__rd = scpu_get_reg(cpu, __isn->rd);			\
		__rs1 = scpu_get_reg(cpu, __isn->rs1);			\
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu->mem, __rs1,		\
				&__isn->imm, __rd, ret);		\
		break;							\
	}								\
	case SIF_OP3_REG:						\
	{								\
		struct sparc_ifmt_op3_reg const *__isn =		\
				to_ifmt(op3_reg, isn);			\
		sreg *__rd, *__rs1, *__rs2;				\
									\
		__rd = scpu_get_reg(cpu, __isn->rd);			\
		__rs1 = scpu_get_reg(cpu, __isn->rs1);			\
		__rs2 = scpu_get_reg(cpu, __isn->rs2);			\
		ISN_EXEC_OP3_MEM_ ## op(sz, cpu->mem, __rs1, __rs2,	\
				__rd, ret);				\
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

/* -------------- Instruction execution ---------------- */

#define ISN_EXEC_ENTRY(i, f) [i] = f
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
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
