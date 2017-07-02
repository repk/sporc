#include <stdlib.h>
#include <endian.h>

#include "utils.h"

#include "cpu.h"
#include "memory.h"
#include "sparc.h"
#include "isn.h"

#define SPARC_NRWIN 32

struct sparc_registers {
	/* Special registers */
	sreg psr;
	sreg tbr;
	sreg wim;
	sreg y;
	sreg pc;
	sreg npc;

	/* generic registers (%g[0-7], %i[0-7], %o[0-7], %l[0-7]) */
	sreg r[8 * (16 * SPARC_NRWIN + 16)];
};

#define CWP(sr) ((sr)->psr & 0x1f)
#define _SREG_IDX(sr, idx) (((idx) < 8) ? (idx) : (8 + (idx) + CWP(sr) * 16))
#define SREG(sr) ((sr)->r[_SREG_IDX(sr, idx)])

#define SPARC_PIPESZ 2
struct sparc_cpu {
	struct cpu cpu;
	union sparc_isn_fill pipeline[SPARC_PIPESZ];
	struct sparc_registers reg;
};

#define to_sparc_cpu(c) (container_of(c, struct sparc_cpu, cpu))

sreg *scpu_get_reg(struct cpu *cpu, off_t ridx)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	if(ridx < 8)
		return &scpu->reg.r[ridx];

	return &scpu->reg.r[CWP(&scpu->reg) * 16 + ridx];
}

static int scpu_fetch(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret = 0;

	sreg npc = scpu->reg.npc;
	ret = memory_fetch_isn32(cpu->mem, npc, &rd);
	if(ret != 0)
		goto exit;

	scpu->pipeline[1].isn.op = (opcode)be32toh(rd);

exit:
	return ret;
}

static int scpu_decode(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	return isn_decode(&scpu->pipeline[0].isn);
}

static int scpu_exec(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	int ret;

	ret = isn_exec(cpu, &scpu->pipeline[0].isn);
	if(ret < 0)
		return ret;

	scpu->pipeline[0].isn.op = scpu->pipeline[1].isn.op;
	scpu->reg.pc = scpu->reg.npc;
	scpu->reg.npc += 4;

	return 0;
}

static int scpu_boot(struct cpu *cpu, uintptr_t addr)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret;

	scpu->reg.pc = addr;
	scpu->reg.npc = addr + 4;

	ret = memory_fetch_isn32(cpu->mem, scpu->reg.pc, &rd);
	if(ret != 0)
		goto exit;

	scpu->pipeline[0].isn.op = (opcode)be32toh(rd);

exit:
	return ret;
}

static struct cpu *scpu_create(char const *args)
{
	struct sparc_cpu *scpu;
	(void)args; /* TODO manage sparc families */

	scpu = calloc(1, sizeof(*scpu));
	if(scpu == NULL)
		return NULL;

	return &scpu->cpu;
}

static void scpu_destroy(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	free(scpu);
}

static struct cpu_ops const spops = {
	.create = scpu_create,
	.destroy = scpu_destroy,
	.boot = scpu_boot,
	.fetch = scpu_fetch,
	.decode = scpu_decode,
	.exec = scpu_exec,
};

static struct cpu_desc const scpu = {
	.name = "sparc",
	.cops = &spops,
};

CPU_REGISTER(scpu);
