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

#define ISN_EXEC_ENTRY(i, f) [i] = f
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
	ISN_EXEC_ENTRY(SI_SETHI, isn_exec_sethi),
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
