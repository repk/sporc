#include <stdlib.h>
#include <stddef.h>

#include "memory.h"
#include "utils.h"
#include "sparc.h"
#include "isn.h"
#include "trap.h"

struct isn_handler {
	int (*handler)(struct isn_handler const *hdl, struct cpu *cpu,
			struct sparc_isn const *isn);
};

#define INIT_ISN_HDL(op) {						\
	.handler = op,							\
}
#define DEFINE_ISN_HDL(n, op)						\
	static struct isn_handler const isn_handler_ ## n = INIT_ISN_HDL(op)

#define ISN_HDL_ENTRY(i) [SI_ ## i] = &isn_handler_ ## i

/* ----------------- sethi instruction Helpers ------------------- */

static int isn_exec_sethi(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_sethi *i;
	(void)hdl;

	if(isn->fmt != SIF_OP2_SETHI)
		return -1;

	i = to_ifmt(op2_sethi, isn);

	scpu_set_reg(cpu, i->rd, i->imm << 10);
	return 0;
}

DEFINE_ISN_HDL(SETHI, isn_exec_sethi);

/* ----------------- call instruction Helper ------------------- */

static int isn_exec_call(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op1 *i;
	sreg pc;
	(void)hdl;

	if(isn->fmt != SIF_OP1)
		return -1;

	i = to_ifmt(op1, isn);

	pc = scpu_get_pc(cpu);
	scpu_set_reg(cpu, 15, pc);

	scpu_delay_jmp(cpu, pc + (i->disp30 << 2));
	return 0;
}

DEFINE_ISN_HDL(CALL, isn_exec_call);

/* ---------- Define Format 3 instruction dispatch ---------- */

/* Format3 instruction handler */
struct isn_handler_fmt3 {
	struct isn_handler hdl;
	int (*op)(struct isn_handler const *hdl, struct cpu *cpu,
			sridx rd, uint32_t v1, uint32_t v2);
};
#define to_handler_fmt3(h) (container_of(h, struct isn_handler_fmt3, hdl))

/* Define a format3 instruction handler */
#define INIT_ISN_HDL_FMT3(o) {						\
	.hdl = INIT_ISN_HDL(isn_exec_fmt3),				\
	.op = o,							\
}

#define DEFINE_ISN_HDL_FMT3(n, o)					\
	static struct isn_handler_fmt3 const				\
		isn_handler_ ## n = INIT_ISN_HDL_FMT3(o)

#define ISN_HDL_FMT3_ENTRY(i) [SI_ ## i] = &isn_handler_ ## i.hdl

/* Fetch Format3 params for instructions that uses immediate */
static inline void isn_fmt3_get_param_imm(struct cpu *cpu,
		struct sparc_isn const *isn, sridx *rd, uint32_t *v1,
		uint32_t *v2)
{
	struct sparc_ifmt_op3_imm const *i = to_ifmt(op3_imm, isn);

	*rd = i->rd;
	*v1 = scpu_get_reg(cpu, i->rs1);
	*v2 = i->imm;
}

/* Fetch Format3 params for instructions that uses register */
static inline void isn_fmt3_get_param_reg(struct cpu *cpu,
		struct sparc_isn const *isn, sridx *rd, uint32_t *v1,
		uint32_t *v2)
{
	struct sparc_ifmt_op3_reg const *i = to_ifmt(op3_reg, isn);

	*rd = i->rd;
	*v1 = scpu_get_reg(cpu, i->rs1);
	*v2 = scpu_get_reg(cpu, i->rs2);
}

/* Fetch Format3 params then call the proper instruction operation */
static int isn_exec_fmt3(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct isn_handler_fmt3 const *fh = to_handler_fmt3(hdl);
	sridx rd;
	uint32_t v1, v2;
	int ret;

	switch(isn->fmt) {
	case SIF_OP3_IMM:
		isn_fmt3_get_param_imm(cpu, isn, &rd, &v1, &v2);
		break;
	case SIF_OP3_REG:
		isn_fmt3_get_param_reg(cpu, isn, &rd, &v1, &v2);
		break;
	default:
		ret = -1;
		goto out;
	}

	ret = fh->op(hdl, cpu, rd, v1, v2);
out:
	return ret;
}

/* ---------------- jumpl instruction handler ------------------ */

static int isn_exec_jmpl(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	/* TODO see JMPL ASI when RETT is on delay slot (pg 126) */
	if((v1 + v2) & 0x3) {
		scpu_trap(cpu, ST_MEM_UNALIGNED);
		goto out;
	}

	scpu_set_reg(cpu, rd, scpu_get_pc(cpu));
	scpu_delay_jmp(cpu, v1 + v2);
out:
	return 0;
}

DEFINE_ISN_HDL_FMT3(JMPL, isn_exec_jmpl);

/* ---------------- rett instruction handler ------------------ */

static int isn_exec_rett(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	(void)rd;

	scpu_exit_trap(cpu, v1 + v2);
	return 0;
}

DEFINE_ISN_HDL_FMT3(RETT, isn_exec_rett);

/* ----------------- ALU instruction helpers ------------------- */

/* ALU instruction handler */
struct isn_handler_alu {
	struct isn_handler_fmt3 fmt3;
	union {
		int (*icc)(struct isn_handler const *hdl, struct cpu *cpu,
				uint32_t res, uint32_t v1, uint32_t v2);
		int (*icc64)(struct isn_handler const *hdl, struct cpu *cpu,
				uint64_t res, uint32_t v1, uint32_t v2);
	};
};
#define to_handler_alu(h)						\
	(container_of(to_handler_fmt3(h), struct isn_handler_alu, fmt3))

/* Define a ALU instruction handler */
#define INIT_ISN_HDL_ALU(o, fun, cc) {					\
	.fmt3 = INIT_ISN_HDL_FMT3(o),					\
	.fun = cc,							\
}

#define DEFINE_ISN_HDL_ALU(n, o, cc)					\
	static struct isn_handler_alu const				\
		isn_handler_ ## n = INIT_ISN_HDL_ALU(o, icc, cc)

#define DEFINE_ISN_HDL_ALU64(n, o, cc)					\
	static struct isn_handler_alu const				\
		isn_handler_ ## n = INIT_ISN_HDL_ALU(o, icc64, cc)

/* Compute simple N and Z flags */
static int isn_alu_icc_nz(struct isn_handler const *hdl, struct cpu *cpu,
		uint32_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	(void)v1;
	(void)v2;
	scpu_set_cc_n(cpu, (res >> 31) & 0x1);
	scpu_set_cc_z(cpu, ((res == 0) ? 1 : 0));
	return 0;
}

#define ISN_HDL_ALU_ENTRY(i) [SI_ ## i] = &isn_handler_ ## i.fmt3.hdl

/* Define both ISN and ISNcc at once */
#define DEFINE_ISN_HDL_ALUcc(n, o, cc)					\
	DEFINE_ISN_HDL_ALU(n, o, NULL);					\
	DEFINE_ISN_HDL_ALU(n ## CC, o, cc)

#define DEFINE_ISN_HDL_ALUcc64(n, o, cc)				\
	DEFINE_ISN_HDL_ALU64(n, o, NULL);				\
	DEFINE_ISN_HDL_ALU64(n ## CC, o, cc)

#define ISN_HDL_ALUcc_ENTRY(i)						\
	ISN_HDL_ALU_ENTRY(i),						\
	ISN_HDL_ALU_ENTRY(i ## CC)

#define ISN_HDL_ALUcc64_ENTRY(i) ISN_HDL_ALUcc_ENTRY(i)

/* --------------- Simple ALU instruction helpers ----------------- */

/* Simple ALU instruction handler */
struct isn_handler_simple_alu {
	struct isn_handler_alu alu;
	uint32_t (*op)(uint32_t v1, uint32_t v2);
};
#define to_handler_simple_alu(h)					\
	(container_of(to_handler_alu(h), struct isn_handler_simple_alu, alu))

/* Define a simple ALU instruction handler */
#define INIT_ISN_HDL_SIMPLE_ALU(o, cc) {				\
	.alu = INIT_ISN_HDL_ALU(isn_exec_simple_alu, icc, cc),		\
	.op = o,							\
}

#define DEFINE_ISN_HDL_SIMPLE_ALU(n, o, cc)				\
	static struct isn_handler_simple_alu const			\
	isn_handler_ ## n = INIT_ISN_HDL_SIMPLE_ALU(o, cc)

#define ISN_HDL_SIMPLE_ALU_ENTRY(i)					\
	[SI_ ## i] = &isn_handler_ ## i.alu.fmt3.hdl

/* Simple alu handler */
static int isn_exec_simple_alu(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_simple_alu *ah = to_handler_simple_alu(hdl);
	uint32_t res;

	res = ah->op(v1, v2);
	if(ah->alu.icc)
		ah->alu.icc(hdl, cpu, res, v1, v2);

	scpu_set_reg(cpu, rd, res);

	return 0;
}

/* Define both ISN and ISNcc at once */
#define DEFINE_ISN_HDL_SIMPLE_ALUcc(n, o, cc)				\
	DEFINE_ISN_HDL_SIMPLE_ALU(n, o, NULL);				\
	DEFINE_ISN_HDL_SIMPLE_ALU(n ## CC, o, cc)

#define DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(n, o)				\
	DEFINE_ISN_HDL_SIMPLE_ALUcc(n, o, isn_alu_icc_nz)

#define ISN_HDL_SIMPLE_ALUcc_ENTRY(n)					\
	ISN_HDL_SIMPLE_ALU_ENTRY(n),					\
	ISN_HDL_SIMPLE_ALU_ENTRY(n ## CC)

/* ----------------- Logical instruction ------------------- */

static uint32_t isn_exec_or(uint32_t v1, uint32_t v2)
{
	return v1 | v2;
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(OR, isn_exec_or);

static uint32_t isn_exec_orn(uint32_t v1, uint32_t v2)
{
	return v1 | (~v2);
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(ORN, isn_exec_orn);

static uint32_t isn_exec_and(uint32_t v1, uint32_t v2)
{
	return v1 & v2;
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(AND, isn_exec_and);

static uint32_t isn_exec_andn(uint32_t v1, uint32_t v2)
{
	return v1 & (~v2);
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(ANDN, isn_exec_andn);

static uint32_t isn_exec_xor(uint32_t v1, uint32_t v2)
{
	return v1 ^ v2;
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(XOR, isn_exec_xor);

static uint32_t isn_exec_xnor(uint32_t v1, uint32_t v2)
{
	return v1 ^ (~v2);
}
DEFINE_ISN_HDL_SIMPLE_ALUcc_NZ(XNOR, isn_exec_xnor);

static int isn_exec_sll(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	scpu_set_reg(cpu, rd, v1 << (v2 & 0x1f));
	return 0;
}
DEFINE_ISN_HDL_ALU(SLL, isn_exec_sll, NULL);

static int isn_exec_srl(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	scpu_set_reg(cpu, rd, v1 >> (v2 & 0x1f));
	return 0;
}
DEFINE_ISN_HDL_ALU(SRL, isn_exec_srl, NULL);

static int isn_exec_sra(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	scpu_set_reg(cpu, rd, ((int32_t)v1) >> (v2 & 0x1f));
	return 0;
}
DEFINE_ISN_HDL_ALU(SRA, isn_exec_sra, NULL);

/* --------------- Arithmetic instruction ----------------- */

/* Set proper condition flags from previous addcc instruction */
static int isn_alu_icc_add(struct isn_handler const *hdl, struct cpu *cpu,
		uint32_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	isn_alu_icc_nz(hdl, cpu, res, v1, v2);

	if((!(((v1 >> 31) & 0x1) ^ ((v2 >> 31) & 0x1))) &&
		(((v1 >> 31) & 0x1) ^ ((res >> 31) & 0x1)))
		scpu_set_cc_v(cpu, 1);
	else
		scpu_set_cc_v(cpu, 0);

	if((((v1 >> 31) & 0x1) && (((v2 >> 31) & 0x1))) ||
		((!((res >> 31) & 0x1)) && (((v1 >> 31) & 0x1) ||
			((v2 >> 31) & 0x1))))
		scpu_set_cc_c(cpu, 1);
	else
		scpu_set_cc_c(cpu, 0);

	return 0;
}

static uint32_t isn_exec_add(uint32_t v1, uint32_t v2)
{
	return v1 + v2;
}
DEFINE_ISN_HDL_SIMPLE_ALUcc(ADD, isn_exec_add, isn_alu_icc_add);

/* Set proper condition flags from previous subcc instruction */
static int isn_alu_icc_sub(struct isn_handler const *hdl, struct cpu *cpu,
		uint32_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	isn_alu_icc_nz(hdl, cpu, res, v1, v2);
	if((((v1 >> 31) & 0x1) ^ ((v2 >> 31) & 0x1)) &&
		(!(((v2 >> 31) & 0x1) ^ ((res >> 31) & 0x1))))
		scpu_set_cc_v(cpu, 1);
	else
		scpu_set_cc_v(cpu, 0);

	if(((!((v1 >> 31) & 0x1)) && ((v2 >> 31) & 0x1)) ||
		(((v2 >> 31) & 0x1) && ((!((v1 >> 31) & 0x1)) ||
			((v2 >> 31) & 0x1))))
		scpu_set_cc_c(cpu, 1);
	else
		scpu_set_cc_c(cpu, 0);

	return 0;
}

static uint32_t isn_exec_sub(uint32_t v1, uint32_t v2)
{
	return v1 - v2;
}
DEFINE_ISN_HDL_SIMPLE_ALUcc(SUB, isn_exec_sub, isn_alu_icc_sub);

static int isn_exec_mulscc(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	sreg y;
	uint32_t tmp = (v1 >> 1) |
		((scpu_get_cc_v(cpu) & scpu_get_cc_n(cpu)) << 31);
	uint32_t add;

	/* Step add */
	scpu_get_asr(cpu, 0, &y);
	if(y & 0x1) {
		add = isn_exec_add(tmp, v2);
		isn_alu_icc_add(hdl, cpu, add, tmp, v2);
	} else {
		add = isn_exec_add(tmp, 0);
		isn_alu_icc_add(hdl, cpu, add, tmp, 0);
	}

	/* Update y register */
	y = (((uint32_t)y) >> 1) | ((v1 & 0x1) << 31);
	scpu_set_asr(cpu, 0, y, 0);
	scpu_set_reg(cpu, rd, add);
	return 0;
}
DEFINE_ISN_HDL_ALU(MULSCC, isn_exec_mulscc, NULL);

/* Set N and Z flags and reset V and C */
static int isn_alu_icc64_mul(struct isn_handler const *hdl, struct cpu *cpu,
		uint64_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	isn_alu_icc_nz(hdl, cpu, (uint32_t)res, v1, v2);
	scpu_set_cc_v(cpu, 0);
	scpu_set_cc_c(cpu, 0);
	return 0;
}

static int isn_alu_icc64_udiv(struct isn_handler const *hdl, struct cpu *cpu,
		uint64_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	isn_alu_icc_nz(hdl, cpu, (uint32_t)res, v1, v2);
	scpu_set_cc_v(cpu, ((res >> 32) == 0) ? 0 : 1);
	scpu_set_cc_c(cpu, 0);
	return 0;
}

static int isn_alu_icc64_sdiv(struct isn_handler const *hdl, struct cpu *cpu,
		uint64_t res, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	isn_alu_icc_nz(hdl, cpu, (uint32_t)res, v1, v2);
	scpu_set_cc_v(cpu, (((res >> 32) == 0) &&
				((res >> 32) != 0xffffffff)) ? 0 : 1);
	scpu_set_cc_c(cpu, 0);
	return 0;
}

static int isn_exec_umul(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_alu *ah = to_handler_alu(hdl);
	uint64_t res = ((uint64_t)v1) * v2;

	scpu_set_reg(cpu, rd, res & 0xffffffff);
	scpu_set_asr(cpu, 0, res >> 32, 0);
	if(ah->icc64)
		ah->icc64(hdl, cpu, res, v1, v2);

	return 0;
}
DEFINE_ISN_HDL_ALUcc64(UMUL, isn_exec_umul, isn_alu_icc64_mul);

static int isn_exec_smul(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_alu *ah = to_handler_alu(hdl);
	uint64_t res = ((int64_t)((int32_t)v1)) * ((int32_t)v2);

	scpu_set_reg(cpu, rd, res & 0xffffffff);
	scpu_set_asr(cpu, 0, res >> 32, 0);
	if(ah->icc64)
		ah->icc64(hdl, cpu, res, v1, v2);

	return 0;
}
DEFINE_ISN_HDL_ALUcc64(SMUL, isn_exec_smul, isn_alu_icc64_mul);

static int isn_exec_udiv(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_alu *ah = to_handler_alu(hdl);
	uint64_t res, dividend;
	uint32_t y;

	scpu_get_asr(cpu, 0, &y);
	dividend = (((uint64_t)y) << 32) | v1;

	res = dividend / v2;

	if(ah->icc64)
		ah->icc64(hdl, cpu, res, v1, v2);

	if(res >> 32)
		res = 0xffffffff;

	scpu_set_reg(cpu, rd, res & 0xffffffff);
	return 0;
}
DEFINE_ISN_HDL_ALUcc64(UDIV, isn_exec_udiv, isn_alu_icc64_udiv);

static int isn_exec_sdiv(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_alu *ah = to_handler_alu(hdl);
	int64_t res, dividend;
	uint32_t y;

	scpu_get_asr(cpu, 0, &y);
	dividend = (((uint64_t)y) << 32) | v1;

	res = dividend / ((int32_t)v2);

	if(ah->icc64)
		ah->icc64(hdl, cpu, res, v1, v2);

	if(((res >> 32) != 0) && ((res >> 32) != 0xffffffff))
		res = (res > 0) ? 0x7fffffff : 0x80000000;

	scpu_set_reg(cpu, rd, res & 0xffffffff);
	return 0;
}
DEFINE_ISN_HDL_ALUcc64(SDIV, isn_exec_sdiv, isn_alu_icc64_sdiv);

/* -------------- Memory instruction helpers ---------------- */

/* Memory instruction handler */
struct isn_handler_mem {
	struct isn_handler_fmt3 fmt3;
	int (*op)(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2);
	size_t sz;
};

#define to_handler_mem(h)						\
	(container_of(to_handler_fmt3(h), struct isn_handler_mem, fmt3))

/* Define a Memory instruction handler */
#define INIT_ISN_HDL_MEM(o, s) {					\
	.fmt3 = INIT_ISN_HDL_FMT3(isn_exec_mem),			\
	.op = o,							\
	.sz = s,							\
}

#define DEFINE_ISN_HDL_MEM(n, o, s)					\
	static struct isn_handler_mem const				\
		isn_handler_ ## n = INIT_ISN_HDL_MEM(o, s)

#define ISN_HDL_MEM_ENTRY(i)						\
	[SI_ ## i] = &isn_handler_ ## i.fmt3.hdl

/* Check address is aligned on a power of two bit size */
#define ISN_MEM_ALIGNSZ(a, s) (((a) & (((s) >> 3) - 1)) == 0)

/* Dispatch a memory load/store instruction */
static int isn_exec_mem(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_mem *mh = to_handler_mem(hdl);
	int ret;

	if(!ISN_MEM_ALIGNSZ(v1 + v2, mh->sz)) {
		scpu_trap(cpu, ST_MEM_UNALIGNED);
		goto out;
	}

	ret = mh->op(cpu, rd, v1, v2);
	if(ret != 0)
		scpu_trap(cpu, ST_DACCESS_EXCEP);
out:
	return 0;
}

static int isn_exec_ldsb(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret;
	uint8_t d;

	ret = memory_read8(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;

	scpu_set_reg(cpu, rd, sign_ext(d, 7));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LDSB, isn_exec_ldsb, 8);

static int isn_exec_ldsh(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret;
	uint16_t d;

	ret = memory_read16(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;

	scpu_set_reg(cpu, rd, sign_ext(be16toh(d), 15));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LDSH, isn_exec_ldsh, 16);

static int isn_exec_ldub(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret;
	uint8_t d;

	ret = memory_read8(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;

	scpu_set_reg(cpu, rd, d);
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LDUB, isn_exec_ldub, 8);

static int isn_exec_lduh(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret;
	uint16_t d;

	ret = memory_read16(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;

	scpu_set_reg(cpu, rd, be16toh(d));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LDUH, isn_exec_lduh, 16);

static int isn_exec_ld(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret;
	uint32_t d;

	ret = memory_read32(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;

	scpu_set_reg(cpu, rd, be32toh(d));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LD, isn_exec_ld, 32);

static int isn_exec_ldd(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret = 0;
	uint32_t d;

	if(rd & 0x1) {
		scpu_trap(cpu, ST_ILL_ISN);
		goto out;
	}

	ret = memory_read32(cpu->mem, ((uintptr_t)v1) + v2, &d);
	if(ret)
		goto out;
	scpu_set_reg(cpu, rd, be32toh(d));

	ret = memory_read32(cpu->mem, ((uintptr_t)v1) + v2 + 4, &d);
	if(ret)
		goto out;
	scpu_set_reg(cpu, rd + 1, be32toh(d));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(LDD, isn_exec_ldd, 64);

static int isn_exec_stb(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	return memory_write8(cpu->mem, ((uintptr_t)v1) + v2,
			scpu_get_reg(cpu, rd));
}
DEFINE_ISN_HDL_MEM(STB, isn_exec_stb, 8);

static int isn_exec_sth(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	return memory_write16(cpu->mem, ((uintptr_t)v1) + v2,
			htobe16(scpu_get_reg(cpu, rd)));
}
DEFINE_ISN_HDL_MEM(STH, isn_exec_sth, 16);

static int isn_exec_st(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	return memory_write32(cpu->mem, ((uintptr_t)v1) + v2,
			htobe32(scpu_get_reg(cpu, rd)));
}
DEFINE_ISN_HDL_MEM(ST, isn_exec_st, 32);

static int isn_exec_std(struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	int ret = 0;

	if(rd & 0x1) {
		scpu_trap(cpu, ST_ILL_ISN);
		goto out;
	}
	ret = memory_write32(cpu->mem, ((uintptr_t)v1) + v2,
			htobe32(scpu_get_reg(cpu, rd)));
	if(ret != 0)
		goto out;

	ret = memory_write32(cpu->mem, ((uintptr_t)v1) + v2 + 4,
			htobe32(scpu_get_reg(cpu, rd + 1)));
out:
	return ret;
}
DEFINE_ISN_HDL_MEM(STD, isn_exec_std, 64);

/* ---------------------- Icc test -------------------------- */

/* Icc instruction handler for 2nd opcode format */
struct isn_handler_icc {
	struct isn_handler hdl;
	int (*test)(struct cpu *cpu);
};
#define to_handler_icc(h) (container_of(h, struct isn_handler_icc, hdl))

/* Define a Icc instruction handler */
#define INIT_ISN_HDL_ICC(o, cc) {					\
	.hdl = INIT_ISN_HDL(o),						\
	.test = isn_icc_op_ ## cc,					\
}

#define DEFINE_ISN_HDL_ICC(n, o, cc)					\
	static struct isn_handler_icc const				\
		isn_handler_ ## n = INIT_ISN_HDL_ICC(o, cc)

#define ISN_HDL_ICC_ENTRY(i) [SI_ ## i] = &isn_handler_ ## i.hdl

/* Icc instruction handler for 3rd opcode format */
struct isn_handler_fmt3_icc {
	struct isn_handler hdl;
	int (*op)(struct isn_handler const *hdl, struct cpu *cpu,
			uint32_t v1, uint32_t v2);
	int (*test)(struct cpu *cpu);
};
#define to_handler_fmt3_icc(h)						\
	(container_of(h, struct isn_handler_fmt3_icc, hdl))

/* Define a 3rd format Icc instruction handler */
#define INIT_ISN_HDL_FMT3_ICC(o, cc) {					\
	.hdl = INIT_ISN_HDL(isn_exec_fmt3_icc),				\
	.op = o,							\
	.test = isn_icc_op_ ## cc,					\
}

#define DEFINE_ISN_HDL_FMT3_ICC(n, o, cc)				\
	static struct isn_handler_fmt3_icc const			\
		isn_handler_ ## n = INIT_ISN_HDL_FMT3_ICC(o, cc)

#define ISN_HDL_FMT3_ICC_ENTRY(i) [SI_ ## i] = &isn_handler_ ## i.hdl

/* Fetch ICC Format3 params for instructions that uses immediate */
static inline void isn_fmt3_icc_get_param_imm(struct cpu *cpu,
		struct sparc_isn const *isn, uint32_t *v1, uint32_t *v2)
{
	struct sparc_ifmt_op3_icc_imm const *i = to_ifmt(op3_icc_imm, isn);

	*v1 = scpu_get_reg(cpu, i->rs1);
	*v2 = i->imm;
}

/* Fetch ICC Format3 params for instructions that uses register */
static inline void isn_fmt3_icc_get_param_reg(struct cpu *cpu,
		struct sparc_isn const *isn, uint32_t *v1, uint32_t *v2)
{
	struct sparc_ifmt_op3_icc_reg const *i = to_ifmt(op3_icc_reg, isn);

	*v1 = scpu_get_reg(cpu, i->rs1);
	*v2 = scpu_get_reg(cpu, i->rs2);
}

/* Fetch ICC Format3 params then call the proper instruction operation */
static int isn_exec_fmt3_icc(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct isn_handler_fmt3_icc const *fh = to_handler_fmt3_icc(hdl);
	uint32_t v1, v2;
	int ret;

	switch(isn->fmt) {
	case SIF_OP3_ICC_IMM:
		isn_fmt3_icc_get_param_imm(cpu, isn, &v1, &v2);
		break;
	case SIF_OP3_ICC_REG:
		isn_fmt3_icc_get_param_reg(cpu, isn, &v1, &v2);
		break;
	default:
		ret = -1;
		goto out;
	}

	ret = fh->op(hdl, cpu, v1, v2);
out:
	return ret;
}

/* Define all condition flags test functions */

static inline int isn_icc_op_a(struct cpu *c)
{
	(void)c;
	return 1;
}

static inline int isn_icc_op_n(struct cpu *c)
{
	(void)c;
	return 0;
}

static inline int isn_icc_op_ne(struct cpu *c)
{
	return !scpu_get_cc_z(c);
}

static inline int isn_icc_op_e(struct cpu *c)
{
	return scpu_get_cc_z(c);
}

static inline int isn_icc_op_g(struct cpu *c)
{
	return !(scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c)));
}

static inline int isn_icc_op_le(struct cpu *c)
{
	return scpu_get_cc_z(c) || (scpu_get_cc_n(c) ^ scpu_get_cc_v(c));
}

static inline int isn_icc_op_ge(struct cpu *c)
{
	return !(scpu_get_cc_n(c) ^ scpu_get_cc_v(c));
}

static inline int isn_icc_op_l(struct cpu *c)
{
	return scpu_get_cc_n(c) ^ scpu_get_cc_v(c);
}

static inline int isn_icc_op_gu(struct cpu *c)
{
	return !(scpu_get_cc_c(c) || scpu_get_cc_z(c));
}

static inline int isn_icc_op_leu(struct cpu *c)
{
	return scpu_get_cc_c(c) || scpu_get_cc_z(c);
}

static inline int isn_icc_op_cc(struct cpu *c)
{
	return !scpu_get_cc_c(c);
}

static inline int isn_icc_op_cs(struct cpu *c)
{
	return scpu_get_cc_c(c);
}

static inline int isn_icc_op_pos(struct cpu *c)
{
	return !scpu_get_cc_n(c);
}

static inline int isn_icc_op_neg(struct cpu *c)
{
	return scpu_get_cc_n(c);
}

static inline int isn_icc_op_vc(struct cpu *c)
{
	return !scpu_get_cc_v(c);
}

static inline int isn_icc_op_vs(struct cpu *c)
{
	return scpu_get_cc_v(c);
}

/* ------------------- Bicc Instructions -------------------- */

/* BA handles annul bit differently than the other Bicc */
static int isn_exec_ba(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);
	sreg pc;
	(void)hdl;

	if(isn->fmt != SIF_OP2_BICC)
		return -1;

	pc = scpu_get_pc(cpu);
	scpu_delay_jmp(cpu, pc + (i->disp << 2));

	if(i->a)
		scpu_annul_delay_slot(cpu);
	return 0;
}
DEFINE_ISN_HDL(BA, isn_exec_ba);

/* Common handler for other Bicc */
static int isn_exec_bicc(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op2_bicc const *i = to_ifmt(op2_bicc, isn);
	struct isn_handler_icc const *ih = to_handler_icc(hdl);
	sreg pc;

	if(isn->fmt != SIF_OP2_BICC)
		return -1;

	if(!ih->test(cpu)) {
		if(i->a)
			scpu_annul_delay_slot(cpu);
		goto out;
	}
	pc = scpu_get_pc(cpu);
	scpu_delay_jmp(cpu, pc + (i->disp << 2));

out:
	return 0;
}

#define DEFINE_ISN_HDL_BICC(n, cc) DEFINE_ISN_HDL_ICC(n, isn_exec_bicc, cc)
#define ISN_HDL_BICC_ENTRY(n) ISN_HDL_ICC_ENTRY(n)

/* Define all Bicc instructions except ba that is defined above */
DEFINE_ISN_HDL_BICC(BN, n);
DEFINE_ISN_HDL_BICC(BNE, ne);
DEFINE_ISN_HDL_BICC(BE, e);
DEFINE_ISN_HDL_BICC(BG, g);
DEFINE_ISN_HDL_BICC(BLE, le);
DEFINE_ISN_HDL_BICC(BGE, ge);
DEFINE_ISN_HDL_BICC(BL, l);
DEFINE_ISN_HDL_BICC(BGU, gu);
DEFINE_ISN_HDL_BICC(BLEU, leu);
DEFINE_ISN_HDL_BICC(BCC, cc);
DEFINE_ISN_HDL_BICC(BCS, cs);
DEFINE_ISN_HDL_BICC(BPOS, pos);
DEFINE_ISN_HDL_BICC(BNEG, neg);
DEFINE_ISN_HDL_BICC(BVC, vc);
DEFINE_ISN_HDL_BICC(BVS, vs);

/* ---------- Window isn (save/restore) execution ------------ */

static int isn_exec_save(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	scpu_window_save(cpu);
	scpu_set_reg(cpu, rd, v1 + v2);
	return 0;
}
DEFINE_ISN_HDL_FMT3(SAVE, isn_exec_save);

static int isn_exec_restore(struct isn_handler const *hdl, struct cpu *cpu,
		sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;

	scpu_window_restore(cpu);
	scpu_set_reg(cpu, rd, v1 + v2);
	return 0;
}
DEFINE_ISN_HDL_FMT3(RESTORE, isn_exec_restore);

/* -------------- Trap isntruction execution -----------------*/

/* Common handler for all Ticc instructions */
static int isn_exec_ticc(struct isn_handler const *hdl, struct cpu *cpu,
		uint32_t v1, uint32_t v2)
{
	struct isn_handler_fmt3_icc const *ih = to_handler_fmt3_icc(hdl);

	if(!ih->test(cpu))
		goto out;

	scpu_trap(cpu, 128 + ((v1 + v2)  & (0x7f)));
out:
	return 0;
}

#define DEFINE_ISN_HDL_TICC(n, cc)					\
	DEFINE_ISN_HDL_FMT3_ICC(n, isn_exec_ticc, cc)
#define ISN_HDL_TICC_ENTRY(n) ISN_HDL_FMT3_ICC_ENTRY(n)

DEFINE_ISN_HDL_TICC(TA, a);
DEFINE_ISN_HDL_TICC(TN, e);
DEFINE_ISN_HDL_TICC(TNE, ne);
DEFINE_ISN_HDL_TICC(TE, e);
DEFINE_ISN_HDL_TICC(TG, g);
DEFINE_ISN_HDL_TICC(TLE, le);
DEFINE_ISN_HDL_TICC(TGE, ge);
DEFINE_ISN_HDL_TICC(TL, l);
DEFINE_ISN_HDL_TICC(TGU, gu);
DEFINE_ISN_HDL_TICC(TLEU, leu);
DEFINE_ISN_HDL_TICC(TCC, cc);
DEFINE_ISN_HDL_TICC(TCS, cs);
DEFINE_ISN_HDL_TICC(TPOS, pos);
DEFINE_ISN_HDL_TICC(TNEG, neg);
DEFINE_ISN_HDL_TICC(TVC, vc);
DEFINE_ISN_HDL_TICC(TVS, vs);

/* -------------- Specific register instruction ------------- */

/* SREG instruction handler */
struct isn_handler_sreg {
	struct isn_handler_fmt3 fmt3;
	union {
		int (*simple_wr)(struct cpu *cpu, sreg val);
		int (*simple_rd)(struct cpu *cpu, sreg *val);
	};
};

#define to_handler_sreg(h)						\
	(container_of(to_handler_fmt3(h), struct isn_handler_sreg, fmt3))

/* Define a sreg instruction handler */
#define _INIT_ISN_HDL_SREG(type, f) {					\
	.fmt3 = INIT_ISN_HDL_FMT3(isn_exec_sreg_ ## type),		\
	.type = f,							\
}

#define _DEFINE_ISN_HDL_SREG(n, type, f)				\
	static struct isn_handler_sreg const				\
		isn_handler_ ## n = _INIT_ISN_HDL_SREG(type, f)

#define _ISN_HDL_SREG_ENTRY(i)						\
	[SI_ ## i] = &isn_handler_ ## i.fmt3.hdl

#define DEFINE_ISN_HDL_SREG(n, sreg)					\
	_DEFINE_ISN_HDL_SREG(WR ## n, simple_wr, scpu_set_ ## sreg);	\
	_DEFINE_ISN_HDL_SREG(RD ## n, simple_rd, scpu_get_ ## sreg)

#define ISN_HDL_SREG_ENTRY(i)						\
	_ISN_HDL_SREG_ENTRY(WR ## i),					\
	_ISN_HDL_SREG_ENTRY(RD ## i)

#define DEFINE_ISN_HDL_ASR(n)						\
	DEFINE_ISN_HDL_FMT3(WR ## n, isn_exec_sreg_asr_wr);		\
	DEFINE_ISN_HDL(RD ## n, isn_exec_sreg_asr_rd)

#define ISN_HDL_ASR_ENTRY(i)						\
	ISN_HDL_FMT3_ENTRY(WR ## i),					\
	ISN_HDL_ENTRY(RD ## i)

/* Simple special registers (psr, wim, tbr) write handler */
static int isn_exec_sreg_simple_wr(struct isn_handler const *hdl,
		struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_sreg const *sh = to_handler_sreg(hdl);
	(void)rd;

	sh->simple_wr(cpu, v1 ^ v2);
	return 0;
}

/* Simple special registers (psr, wim, tbr) read handler */
static int isn_exec_sreg_simple_rd(struct isn_handler const *hdl,
		struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	struct isn_handler_sreg const *sh = to_handler_sreg(hdl);
	sreg val;
	(void) v1;
	(void) v2;

	if(sh->simple_rd(cpu, &val) != 0)
		goto out;

	scpu_set_reg(cpu, rd, val);
out:
	return 0;
}

/* ASR special registers write handler */
static int isn_exec_sreg_asr_wr(struct isn_handler const *hdl,
		struct cpu *cpu, sridx rd, uint32_t v1, uint32_t v2)
{
	(void)hdl;
	scpu_set_asr(cpu, rd, v1, v2);
	return 0;
}

/* ASR special registers read handler */
static int isn_exec_sreg_asr_rd(struct isn_handler const *hdl, struct cpu *cpu,
		struct sparc_isn const *isn)
{
	struct sparc_ifmt_op3_reg *i;
	sreg val;
	(void)hdl;

	if(isn->fmt != SIF_OP3_REG)
		return -1;

	i = to_ifmt(op3_reg, isn);

	if(scpu_get_asr(cpu, i->rs1, &val) != 0)
		goto out;

	scpu_set_reg(cpu, i->rd, val);
out:
	return 0;
}

/* Define all specific register instructions handlers */
DEFINE_ISN_HDL_ASR(ASR);
DEFINE_ISN_HDL_SREG(PSR, psr);
DEFINE_ISN_HDL_SREG(WIM, wim);
DEFINE_ISN_HDL_SREG(TBR, tbr);

/* ------------ Instruction execution dispatch -------------- */

/* Instruction handler dispatch array */
static struct isn_handler const *_exec_isn[] = {
	ISN_HDL_ENTRY(SETHI),
	ISN_HDL_ENTRY(CALL),
	ISN_HDL_FMT3_ENTRY(JMPL),
	ISN_HDL_FMT3_ENTRY(RETT),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(AND),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(ANDN),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(OR),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(ORN),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(XOR),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(XNOR),
	ISN_HDL_ALU_ENTRY(SLL),
	ISN_HDL_ALU_ENTRY(SRL),
	ISN_HDL_ALU_ENTRY(SRA),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(ADD),
	ISN_HDL_SIMPLE_ALUcc_ENTRY(SUB),
	ISN_HDL_ALU_ENTRY(MULSCC),
	ISN_HDL_ALUcc64_ENTRY(UMUL),
	ISN_HDL_ALUcc64_ENTRY(SMUL),
	ISN_HDL_ALUcc64_ENTRY(UDIV),
	ISN_HDL_ALUcc64_ENTRY(SDIV),
	ISN_HDL_MEM_ENTRY(LDSB),
	ISN_HDL_MEM_ENTRY(LDSH),
	ISN_HDL_MEM_ENTRY(LDUB),
	ISN_HDL_MEM_ENTRY(LDUH),
	ISN_HDL_MEM_ENTRY(LD),
	ISN_HDL_MEM_ENTRY(LDD),
	ISN_HDL_MEM_ENTRY(STB),
	ISN_HDL_MEM_ENTRY(STH),
	ISN_HDL_MEM_ENTRY(ST),
	ISN_HDL_MEM_ENTRY(STD),
	ISN_HDL_ENTRY(BA),
	ISN_HDL_BICC_ENTRY(BN),
	ISN_HDL_BICC_ENTRY(BNE),
	ISN_HDL_BICC_ENTRY(BE),
	ISN_HDL_BICC_ENTRY(BG),
	ISN_HDL_BICC_ENTRY(BLE),
	ISN_HDL_BICC_ENTRY(BGE),
	ISN_HDL_BICC_ENTRY(BL),
	ISN_HDL_BICC_ENTRY(BGU),
	ISN_HDL_BICC_ENTRY(BLEU),
	ISN_HDL_BICC_ENTRY(BCC),
	ISN_HDL_BICC_ENTRY(BCS),
	ISN_HDL_BICC_ENTRY(BPOS),
	ISN_HDL_BICC_ENTRY(BNEG),
	ISN_HDL_BICC_ENTRY(BVC),
	ISN_HDL_BICC_ENTRY(BVS),
	ISN_HDL_FMT3_ENTRY(SAVE),
	ISN_HDL_FMT3_ENTRY(RESTORE),
	ISN_HDL_TICC_ENTRY(TA),
	ISN_HDL_TICC_ENTRY(TN),
	ISN_HDL_TICC_ENTRY(TNE),
	ISN_HDL_TICC_ENTRY(TE),
	ISN_HDL_TICC_ENTRY(TG),
	ISN_HDL_TICC_ENTRY(TLE),
	ISN_HDL_TICC_ENTRY(TGE),
	ISN_HDL_TICC_ENTRY(TL),
	ISN_HDL_TICC_ENTRY(TGU),
	ISN_HDL_TICC_ENTRY(TLEU),
	ISN_HDL_TICC_ENTRY(TCC),
	ISN_HDL_TICC_ENTRY(TCS),
	ISN_HDL_TICC_ENTRY(TPOS),
	ISN_HDL_TICC_ENTRY(TNEG),
	ISN_HDL_TICC_ENTRY(TVC),
	ISN_HDL_TICC_ENTRY(TVS),
	ISN_HDL_ASR_ENTRY(ASR),
	ISN_HDL_SREG_ENTRY(PSR),
	ISN_HDL_SREG_ENTRY(WIM),
	ISN_HDL_SREG_ENTRY(TBR),
};

/* Dispatch instruction */
int isn_exec(struct cpu *cpu, struct sparc_isn const *i)
{
	if((i->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[i->id])
		return _exec_isn[i->id]->handler(_exec_isn[i->id], cpu, i);

	return -1;
}
