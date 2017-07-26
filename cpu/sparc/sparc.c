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
	/* pc[0] is pc, pc[1] is npc and pc[2] is filled by branche isn */
	sreg pc[3];

	/* generic registers (%g[0-7], %i[0-7], %o[0-7], %l[0-7]) */
	sreg r[8 * (16 * SPARC_NRWIN + 16)];
};

#define PSR_ICC_OFF_N (23)
#define PSR_ICC_OFF_Z (22)
#define PSR_ICC_OFF_V (21)
#define PSR_ICC_OFF_C (20)
#define PSR_ICC_GET(sr, n) (((sr)->psr >> PSR_ICC_OFF_ ## n) & 0x1)
#define PSR_ICC_SET(sr, n, v)						\
	(((sr)->psr = ((sr)->psr & ~(1 << ((PSR_ICC_OFF_ ## n)))) |	\
		(((v) & 0x1)  << (PSR_ICC_OFF_ ## n))))
#define PSR_CWP(sr) ((sr)->psr & 0x1f)
#define PSR_SET_CWP(sr, v) ((sr)->psr = ((sr)->psr & ~(0x1f)) | ((v) & 0x1f))
#define _SREG_IDX(sr, idx) (((idx) < 8) ? (idx) :			\
		(8 + (idx) + PSR_CWP(sr) * 16))
#define SREG(sr) ((sr)->r[_SREG_IDX(sr, idx)])

#define SPARC_PIPESZ 2
struct sparc_cpu {
	struct cpu cpu;
	/* Cpu instruction pipeline */
	union sparc_isn_fill pipeline[SPARC_PIPESZ];
	struct sparc_registers reg;
	/* Annul next instruction flag */
	uint8_t annul;
};

#define to_sparc_cpu(c) (container_of(c, struct sparc_cpu, cpu))

/**
 * Get a register from its opcode index
 *
 * @param cpu: cpu to fetch register from
 * @param ridx: register index
 * @return: A pointer to the register index
 */
sreg *scpu_get_reg(struct cpu *cpu, off_t ridx)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	if(ridx < 8)
		return &scpu->reg.r[ridx];

	return &scpu->reg.r[PSR_CWP(&scpu->reg) * 16 + ridx];
}

/**
 * Get negative conditional code flag value
 *
 * @param cpu: cpu to get conditional code from
 * @return: Value of conditional code flag
 */
uint8_t scpu_get_cc_n(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return PSR_ICC_GET(&scpu->reg, N);
}

/**
 * Set negative conditional code flag value
 *
 * @param cpu: cpu to set conditional code to
 * @param val: Value of conditional code flag
 */
void scpu_set_cc_n(struct cpu *cpu, uint8_t val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	PSR_ICC_SET(&scpu->reg, N, val);
}

/**
 * Get zero conditional code flag value
 *
 * @param cpu: cpu to get conditional code from
 * @return: Value of conditional code flag
 */
uint8_t scpu_get_cc_z(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return PSR_ICC_GET(&scpu->reg, Z);
}

/**
 * Set zero conditional code flag value
 *
 * @param cpu: cpu to set conditional code to
 * @param val: Value of conditional code flag
 */
void scpu_set_cc_z(struct cpu *cpu, uint8_t val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	PSR_ICC_SET(&scpu->reg, Z, val);
}

/**
 * Get overflow conditional code flag value
 *
 * @param cpu: cpu to get conditional code from
 * @return: Value of conditional code flag
 */
uint8_t scpu_get_cc_v(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return PSR_ICC_GET(&scpu->reg, V);
}

/**
 * Set overflow conditional code flag value
 *
 * @param cpu: cpu to set conditional code to
 * @param val: Value of conditional code flag
 */
void scpu_set_cc_v(struct cpu *cpu, uint8_t val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	PSR_ICC_SET(&scpu->reg, V, val);
}

/**
 * Get carry conditional code flag value
 *
 * @param cpu: cpu to get conditional code from
 * @return: Value of conditional code flag
 */
uint8_t scpu_get_cc_c(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return PSR_ICC_GET(&scpu->reg, C);
}

/**
 * Set carry conditional code flag value
 *
 * @param cpu: cpu to set conditional code to
 * @param val: Value of conditional code flag
 */
void scpu_set_cc_c(struct cpu *cpu, uint8_t val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	PSR_ICC_SET(&scpu->reg, C, val);
}

/**
 * Get PC register value
 *
 * @param cpu: cpu to get PC register from
 * @return: PC reg value
 */
sreg scpu_get_pc(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return scpu->reg.pc[0];
}

/**
 * Get nPC register value
 *
 * @param cpu: cpu to get nPC register from
 * @return: nPC reg value
 */
sreg scpu_get_npc(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return scpu->reg.pc[0];
}

/**
 * Prepare a delay jump
 *
 * @param cpu: cpu that need to jump
 * @return: delay jump address
 */
void scpu_delay_jmp(struct cpu *cpu, uint32_t addr)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	scpu->reg.pc[2] = addr;
}

/**
 * Set the annul delay slot flag
 *
 * @param cpu: cpu that need to cancel its delay slot
 */
void scpu_annul_delay_slot(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	scpu->annul = 1;
}

/**
 * Enter a new register window
 *
 * @param cpu: current cpu
 */
void scpu_window_save(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint8_t cwp = PSR_CWP(&scpu->reg);

	cwp = ((uint8_t)(cwp - 1)) % SPARC_NRWIN;

	/* TODO Trap on window underflow */
	if(scpu->reg.wim & (1 << cwp))
		abort();

	PSR_SET_CWP(&scpu->reg, cwp);
}

/**
 * Exit current register window
 *
 * @param cpu: current cpu
 */
void scpu_window_restore(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint8_t cwp = PSR_CWP(&scpu->reg);

	cwp = (cwp + 1) % SPARC_NRWIN;

	/* TODO Trap on window underflow */
	if(scpu->reg.wim & (1 << cwp))
		abort();

	PSR_SET_CWP(&scpu->reg, cwp);
}

static int scpu_fetch(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret = 0;

	sreg npc = scpu->reg.pc[1];
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

	/* Cancel delay slot */
	if(scpu->annul) {
		scpu->reg.pc[1] = scpu->reg.pc[2];
		scpu->reg.pc[2] += 4;
		ret = scpu_fetch(cpu);
		if(ret < 0)
			return ret;
		scpu->annul = 0;
	}

	/* Move the pipeline to the next instruction */
	scpu->pipeline[0].isn.op = scpu->pipeline[1].isn.op;
	/* Set next instruction PC registers values */
	scpu->reg.pc[0] = scpu->reg.pc[1];
	scpu->reg.pc[1] = scpu->reg.pc[2];
	scpu->reg.pc[2] += 4;

	return 0;
}

static int scpu_boot(struct cpu *cpu, uintptr_t addr)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret;

	/* Initialize PC registers */
	scpu->reg.pc[0] = addr;
	scpu->reg.pc[1] = addr + 4;
	scpu->reg.pc[2] = addr + 8;

	/* TODO initialize special registers */

	/* Prefetch the first instruction */
	ret = memory_fetch_isn32(cpu->mem, scpu->reg.pc[0], &rd);
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
