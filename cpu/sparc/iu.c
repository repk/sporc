#include <stdlib.h>
#include <stddef.h>

#include "memory.h"
#include "utils.h"
#include "sparc.h"
#include "isn.h"

#define ISN_EXEC_ENTRY(i, f) [i] = f
static int (* const _exec_isn[])(struct cpu *cpu, struct sparc_isn const *) = {
};

int isn_exec(struct cpu *cpu, struct sparc_isn const *isn)
{
	if((isn->id < ARRAY_SIZE(_exec_isn)) && _exec_isn[isn->id])
		return _exec_isn[isn->id](cpu, isn);
	return 0;
}
