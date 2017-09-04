#ifndef _ISN_H_
#define _ISN_H_

#include <stdint.h>
#include <endian.h>

#include "cpu.h"

typedef uint32_t opcode;

enum sisn_fmt {
	SIF_UNKNOW = 0,
	SIF_OP1,
	SIF_OP2_SETHI,
	SIF_OP2_BICC,
	SIF_OP3_REG,
	SIF_OP3_IMM,
	SIF_OP3_ICC_REG,
	SIF_OP3_ICC_IMM,
	SIF_OP3_FLOAT,
};

enum sid_isn {
	SI_INVAL = 0,
	SI_SETHI,
	SI_CALL,
	SI_JMPL,
	SI_AND,
	SI_ANDCC,
	SI_ANDN,
	SI_ANDNCC,
	SI_OR,
	SI_ORCC,
	SI_ORN,
	SI_ORNCC,
	SI_XOR,
	SI_XORCC,
	SI_XNOR,
	SI_XNORCC,
	SI_ADD,
	SI_ADDCC,
	SI_SUB,
	SI_SUBCC,
	SI_LDSB,
	SI_LDSH,
	SI_LDUB,
	SI_LDUH,
	SI_LD,
	SI_LDD,
	SI_STB,
	SI_STH,
	SI_ST,
	SI_STD,
	SI_BN,
	SI_BA,
	SI_BNE,
	SI_BE,
	SI_BG,
	SI_BLE,
	SI_BGE,
	SI_BL,
	SI_BGU,
	SI_BLEU,
	SI_BCC,
	SI_BCS,
	SI_BPOS,
	SI_BNEG,
	SI_BVC,
	SI_BVS,
	SI_SAVE,
	SI_RESTORE,
	SI_TA,
	SI_TN,
	SI_TNE,
	SI_TE,
	SI_TG,
	SI_RDPSR,
	SI_WRPSR,
	SI_RDWIM,
	SI_WRWIM,
	SI_RDTBR,
	SI_WRTBR,
};

struct sparc_isn {
	opcode op;
	enum sid_isn id;
	enum sisn_fmt fmt;
};

struct sparc_ifmt_op1 {
	struct sparc_isn isn;
	uint32_t disp30;
};

struct sparc_ifmt_op2_sethi {
	struct sparc_isn isn;
	uint32_t imm;
	uint8_t rd;
};

struct sparc_ifmt_op2_bicc {
	struct sparc_isn isn;
	uint32_t disp;
	uint8_t a;
};

struct sparc_ifmt_op3_reg {
	struct sparc_isn isn;
	uint8_t rd;
	uint8_t rs1;
	uint8_t asi;
	uint8_t rs2;
};

struct sparc_ifmt_op3_imm {
	struct sparc_isn isn;
	uint32_t imm;
	uint8_t rd;
	uint8_t rs1;
};

struct sparc_ifmt_op3_icc_reg {
	struct sparc_isn isn;
	uint8_t rs1;
	uint8_t rs2;
};

struct sparc_ifmt_op3_icc_imm {
	struct sparc_isn isn;
	uint32_t imm;
	uint8_t rs1;
};

struct sparc_ifmt_op3_float {
	struct sparc_isn isn;
	uint8_t rd;
	uint8_t rs2;
	uint16_t rs1;
};

union sparc_isn_fill {
	struct sparc_isn isn;
	struct sparc_ifmt_op1 op1;
	struct sparc_ifmt_op2_sethi op2_sethi;
	struct sparc_ifmt_op2_bicc op2_bicc;
	struct sparc_ifmt_op3_reg op3_reg;
	struct sparc_ifmt_op3_imm op3_imm;
	struct sparc_ifmt_op3_float op3_float;
};

#define to_ifmt(n, i) (container_of(i, struct sparc_ifmt_ ## n, isn))

int isn_decode(struct sparc_isn *isn);
int isn_exec(struct cpu *cpu, struct sparc_isn const *isn);

#endif
