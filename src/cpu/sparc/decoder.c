#include <stdlib.h>

#include "utils.h"

#include "isn.h"

/* Decode flag for type 0 instruction */
static inline uint8_t op_decode_flag0(opcode op)
{
	return (op >> 30) & 0x3;
}

/* Decode flag for type 2 instruction */
static inline uint8_t op_decode_flag2(opcode op)
{
	return (op >> 22) & 0x7;
}

/* Decode flag for type 3 instruction */
static inline uint8_t op_decode_flag3(opcode op)
{
	return (op >> 19) & 0x3f;
}

#define ISN_OP1_DISP(o) ((o) & 0x3fffffff)

static int isn_decode_op1(struct sparc_isn *isn)
{
	struct sparc_ifmt_op1 *i = to_ifmt(op1, isn);

	/* Only Call instruction is of op1 type */
	i->isn.id = SI_CALL;
	i->isn.fmt = SIF_OP1;
	i->disp30 = ISN_OP1_DISP(isn->op);
	return 0;
}

#define ISN_OP2_RD(o) (((o) >> 25) & 0x1f)
#define ISN_OP2_COND(o) (((o) >> 25) & 0xf)
#define ISN_OP2_A(o) (((o) >> 29) & 0x1)
#define ISN_OP2_IMM(o) ((o) & 0x3fffff)
#define ISN_OP2_DISP(o) (sign_ext((o) & 0x3fffff, 22))

static inline int isn_decode_op2_sethi(struct sparc_isn *isn)
{
	struct sparc_ifmt_op2_sethi *i = to_ifmt(op2_sethi, isn);

	i->isn.fmt = SIF_OP2_SETHI;
	i->isn.id = SI_SETHI;
	i->rd = ISN_OP2_RD(isn->op);
	i->imm = ISN_OP2_IMM(isn->op);

	return 0;
}

/**
 * Decode a type 2 branch instruction
 */
static inline int isn_decode_op2_bicc(struct sparc_isn *isn)
{
	/* List of type 2 instruction indexed by its opcode's flag value */
	static enum sid_isn const _op2_bicc_id[] = {
		[8] = SI_BA,
		[0] = SI_BN,
		[9] = SI_BNE,
		[1] = SI_BE,
		[10] = SI_BG,
		[2] = SI_BLE,
		[11] = SI_BGE,
		[3] = SI_BL,
		[12] = SI_BGU,
		[4] = SI_BLEU,
		[13] = SI_BCC,
		[5] = SI_BCS,
		[14] = SI_BPOS,
		[6] = SI_BNEG,
		[15] = SI_BVC,
		[7] = SI_BVS,
	};
	struct sparc_ifmt_op2_bicc *i = to_ifmt(op2_bicc, isn);
	uint8_t cond, annul;

	cond = ISN_OP2_COND(isn->op);
	annul = ISN_OP2_A(isn->op);
	if((cond >= ARRAY_SIZE(_op2_bicc_id)) || (_op2_bicc_id[cond] == 0))
		return -1;

	isn->fmt = SIF_OP2_BICC;
	isn->id = _op2_bicc_id[cond];
	i->a = annul;
	i->disp = ISN_OP2_DISP(isn->op);

	return 0;
}

static int isn_decode_op2(struct sparc_isn *isn)
{
	/**
	 * Type 2 instructions are splitted into sethi and branch instructions
	 */
	static int (* const _decode_op2[])(struct sparc_isn *) = {
		[2] = isn_decode_op2_bicc,
		[4] = isn_decode_op2_sethi,
	};
	uint8_t flag;

	flag = op_decode_flag2(isn->op);
	if((flag >= ARRAY_SIZE(_decode_op2)) || (_decode_op2[flag] == 0))
		return -1;

	return _decode_op2[flag](isn);
}

#define OP3_ICC (SI_TA) /* Temporary set Ticc instruction to ta */
#define ISN_OP3_I(o) ((isn->op >> 13) & 0x1)
#define ISN_OP3_RD(o) (((o) >> 25) & 0x1f)
#define ISN_OP3_RS1(o) (((o) >> 14) & 0x1f)
#define ISN_OP3_IMM(o) sign_ext(o & 0x1fff, 12)
#define ISN_OP3_ASI(o) (((o) >> 5) & 0xff)
#define ISN_OP3_RS2(o) ((o) & 0x1f)
#define ISN_OP3_ICC(o) (((o) >> 25) & 0xf)

/**
 * Decode a type 3 instruction that uses immediate
 */
static int isn_decode_op3_int_imm(struct sparc_isn *isn)
{
	struct sparc_ifmt_op3_imm *i = to_ifmt(op3_imm, isn);

	i->isn.fmt = SIF_OP3_IMM;
	i->rd = ISN_OP3_RD(isn->op);
	i->rs1 = ISN_OP3_RS1(isn->op);
	i->imm = ISN_OP3_IMM(isn->op);

	return 0;
}

/**
 * Decode a type 3 instruction that uses register
 */
static int isn_decode_op3_int_reg(struct sparc_isn *isn)
{
	struct sparc_ifmt_op3_reg *i = to_ifmt(op3_reg, isn);

	i->isn.fmt = SIF_OP3_REG;
	i->rd = ISN_OP3_RD(isn->op);
	i->rs1 = ISN_OP3_RS1(isn->op);
	i->asi = ISN_OP3_ASI(isn->op);
	i->rs2 = ISN_OP3_RS2(isn->op);

	return 0;
}

/** * Decode a type 3 Ticc instruction that uses immediate
 */
static int isn_decode_op3_int_icc_imm(struct sparc_isn *isn)
{
	struct sparc_ifmt_op3_icc_imm *i = to_ifmt(op3_icc_imm, isn);

	i->isn.fmt = SIF_OP3_ICC_IMM;
	i->rs1 = ISN_OP3_RS1(isn->op);
	i->imm = ISN_OP3_IMM(isn->op);

	return 0;
}

/**
 * Decode a type 3 Ticc instruction that uses register
 */
static int isn_decode_op3_int_icc_reg(struct sparc_isn *isn)
{
	struct sparc_ifmt_op3_icc_reg *i = to_ifmt(op3_icc_reg, isn);

	i->isn.fmt = SIF_OP3_ICC_REG;
	i->rs1 = ISN_OP3_RS1(isn->op);
	i->rs2 = ISN_OP3_RS2(isn->op);

	return 0;
}

/**
 * Decode a Ticc instruction
 */
static int isn_decode_op3_icc(struct sparc_isn *isn)
{
	static enum sid_isn const _isn_op3_icc[] = {
		[8] = SI_TA,
	};
	int ret;
	uint8_t cond;

	cond = ISN_OP3_ICC(isn->op);
	if((cond >= ARRAY_SIZE(_isn_op3_icc)) || (_isn_op3_icc[cond] == 0))
		return -1;

	isn->id = _isn_op3_icc[cond];
	if(ISN_OP3_I(isn->op))
		ret = isn_decode_op3_int_icc_imm(isn);
	else
		ret = isn_decode_op3_int_icc_reg(isn);

	return ret;
}

struct _op3_isn_type {
	enum sid_isn id;
	uint8_t isfloat;
};
#define _OP3_ISN_INT(o, i) [o] = { .id = i, }
#define _OP3_ISN_FLOAT(o, i) [o] = { .id = i, .isfloat = 1, }
#define _OP3_ISN_ICC(o) [o] = { .id = OP3_ICC, }

/**
 * Common code for type 3 instruction decoding, splitted in float, immediate and
 * register typed instructions
 */
static inline int isn_decode_op3(struct sparc_isn *isn,
		struct _op3_isn_type const it[], size_t sz)
{
	uint8_t flag;
	int ret;

	flag = op_decode_flag3(isn->op);
	if((flag >= sz) || (it[flag].id == 0))
		return -1;

	isn->id = it[flag].id;

	if(isn->id == OP3_ICC)
		ret = isn_decode_op3_icc(isn);
	else if(it[flag].isfloat)
		ret = -1; /* TODO handle float */
	else if(ISN_OP3_I(isn->op))
		ret = isn_decode_op3_int_imm(isn);
	else
		ret = isn_decode_op3_int_reg(isn);

	return ret;
}

static int isn_decode_op3_2(struct sparc_isn *isn)
{
	/* Type 3-2 instruction list, indexed by their opcode flag value */
	static struct _op3_isn_type const _isn_op3_2[] = {
		_OP3_ISN_INT(1, SI_AND),
		_OP3_ISN_INT(17, SI_ANDCC),
		_OP3_ISN_INT(5, SI_ANDN),
		_OP3_ISN_INT(21, SI_ANDNCC),
		_OP3_ISN_INT(2, SI_OR),
		_OP3_ISN_INT(18, SI_ORCC),
		_OP3_ISN_INT(6, SI_ORN),
		_OP3_ISN_INT(22, SI_ORNCC),
		_OP3_ISN_INT(3, SI_XOR),
		_OP3_ISN_INT(19, SI_XORCC),
		_OP3_ISN_INT(7, SI_XNOR),
		_OP3_ISN_INT(23, SI_XNORCC),
		_OP3_ISN_INT(0, SI_ADD),
		_OP3_ISN_INT(16, SI_ADDCC),
		_OP3_ISN_INT(4, SI_SUB),
		_OP3_ISN_INT(20, SI_SUBCC),
		_OP3_ISN_INT(56, SI_JMPL),
		_OP3_ISN_INT(60, SI_SAVE),
		_OP3_ISN_INT(61, SI_RESTORE),
		_OP3_ISN_ICC(58), /* Ticc */
		_OP3_ISN_INT(41, SI_RDPSR),
		_OP3_ISN_INT(49, SI_WRPSR),
		_OP3_ISN_INT(42, SI_RDWIM),
		_OP3_ISN_INT(50, SI_WRWIM),
		_OP3_ISN_INT(43, SI_RDTBR),
		_OP3_ISN_INT(51, SI_WRTBR),
	};

	return isn_decode_op3(isn, _isn_op3_2, ARRAY_SIZE(_isn_op3_2));
}

static int isn_decode_op3_3(struct sparc_isn *isn)
{
	/* Type 3-3 instruction list, indexed by their opcode flag value */
	static struct _op3_isn_type const _isn_op3_3[] = {
		_OP3_ISN_INT(9, SI_LDSB),
		_OP3_ISN_INT(10, SI_LDSH),
		_OP3_ISN_INT(1, SI_LDUB),
		_OP3_ISN_INT(2, SI_LDUH),
		_OP3_ISN_INT(0, SI_LD),
		_OP3_ISN_INT(3, SI_LDD),
		_OP3_ISN_INT(5, SI_STB),
		_OP3_ISN_INT(6, SI_STH),
		_OP3_ISN_INT(4, SI_ST),
		_OP3_ISN_INT(7, SI_STD),
	};

	return isn_decode_op3(isn, _isn_op3_3, ARRAY_SIZE(_isn_op3_3));
}

int isn_decode(struct sparc_isn *isn)
{
	static int (* const _decode_op0[])(struct sparc_isn *) = {
		[0] = isn_decode_op2,
		[1] = isn_decode_op1,
		[2] = isn_decode_op3_2,
		[3] = isn_decode_op3_3,
	};
	uint8_t flag;

	flag = op_decode_flag0(isn->op);
	if((flag < ARRAY_SIZE(_decode_op0)) && _decode_op0[flag])
		return _decode_op0[flag](isn);

	return 0;
}

