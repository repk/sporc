#include <stdlib.h>
#include <endian.h>

#include "utils.h"
#include "types.h"

#include "cpu/cpu.h"
#include "sparc.h"
#include "isn.h"
#include "trap.h"

#define SPARC_NRWIN 32

struct sparc_registers {
	/* Special registers */
	sreg psr;
	sreg tbr;
	sreg wim;
	sreg y;
	/* pc[0] is pc, pc[1] is npc and pc[2] is filled by branche isn */
	sreg pc[3];

	/*
	 * general purpose registers (%g[1-7], %i[0-7], %o[0-7], %l[0-7])
	 * (%g0 is a special always null register, thus do not need to be stored
	 * in this array)
	 */
	sreg r[7 + (16 * SPARC_NRWIN + 16)];
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
#define PSR_ET(sr) (((sr)->psr >> 5) & 0x1)
#define PSR_SET_ET(sr, v)						\
	((sr)->psr = ((sr)->psr & ~(1 << 5)) | (((v) & 0x1) << 5))
#define PSR_PS(sr) (((sr)->psr >> 6) & 0x1)
#define PSR_SET_PS(sr, v)						\
	((sr)->psr = ((sr)->psr & ~(1 << 6)) | (((v) & 0x1) << 6))
#define PSR_S(sr) (((sr)->psr >> 7) & 0x1)
#define PSR_SET_S(sr, v)						\
	((sr)->psr = ((sr)->psr & ~(1 << 7)) | (((v) & 0x1) << 7))

#define TBR_TT(sr) (((sr)->tbr >> 4) & 0xff)
#define TBR_SET_TT(sr, v)						\
	((sr)->tbr = ((sr)->tbr & ~(0xff0)) | (((v) & 0xff) << 4))

#define _SREG_IDX(sr, idx) (((idx) < 8) ? (idx) :			\
		(8 + (idx) + PSR_CWP(sr) * 16))
#define SREG(sr) ((sr)->r[_SREG_IDX(sr, idx)])

enum scpu_mode {
	SM_ERR,
	SM_EXC,
};

#define SPARC_PIPESZ 2
struct sparc_cpu {
	struct cpu cpu;
	/* Cpu instruction pipeline */
	union sparc_isn_fill pipeline[SPARC_PIPESZ];
	struct sparc_registers reg;
	struct trap_queue tq;
	enum scpu_mode mode;
	/* Annul next instruction flag */
	uint8_t annul;
};

#define to_sparc_cpu(c) (container_of(c, struct sparc_cpu, cpu))

/**
 * Change sparc cpu mode
 *
 * @param cpu: cpu to change the mode
 * @param mode: new cpu mode
 */
static inline void scpu_set_mode(struct cpu *cpu, enum scpu_mode mode)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	scpu->mode = mode;
}

/**
 * Is cpu in error mode
 *
 * @param cpu: cpu to check error mode
 * @return: positive value if cpu is in error mode, 0 otherwise
 */
static inline int scpu_is_error_mode(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	return (scpu->mode == SM_ERR);
}

/**
 * Get a generic register from its opcode index
 *
 * @param cpu: cpu to fetch register from
 * @param ridx: register index
 * @return: Register value
 */
sreg scpu_get_reg(struct cpu *cpu, sridx ridx)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	if(ridx == 0 || ridx >= 32)
		return 0;

	if(ridx < 8)
		return scpu->reg.r[ridx - 1];

	return scpu->reg.r[PSR_CWP(&scpu->reg) * 16 + ridx - 1];
}

/**
 * Set a generic register from its opcode index
 *
 * @param cpu: cpu to fetch register from
 * @param ridx: register index
 * @param val: Value to set the register to
 */
void scpu_set_reg(struct cpu *cpu, sridx ridx, sreg val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	if(ridx == 0 || ridx >= 32)
		return;

	if(ridx < 8)
		scpu->reg.r[ridx - 1] = val;
	else
		scpu->reg.r[PSR_CWP(&scpu->reg) * 16 + ridx - 1] = val;
}

/**
 * Set cpu trap flag (without perm checking)
 *
 * @param cpu: current cpu
 * @param tn: trap number
 */
static void scpu_tflag_set(struct cpu *cpu, uint8_t tn)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	tq_raise(&scpu->tq, tn);
}

/**
 * Fetch the specific PSR register
 *
 * @param cpu: cpu to fetch PSR from
 * @param val: Register read value
 * @return: Register 0 on read success, -1 otherwise
 */
int scpu_get_psr(struct cpu *cpu, sreg *val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Privilege checking */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	/* EC and EF should be masked out */
	*val = (scpu->reg.psr & ~(0x3 << 12));
	return 0;
}

/**
 * Write in specific PSR register
 *
 * @param cpu: cpu to write PSR from
 * @param val: value to write PSR with
 * @return: 0 on success, -1 otherwise
 */
int scpu_set_psr(struct cpu *cpu, sreg val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Privilege checking */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	/* Check CWP */
	if((val & 0x1f) >= SPARC_NRWIN) {
		scpu_tflag_set(cpu, ST_ILL_ISN);
		return -1;
	}

	/**
	 * XXX It's not clear in the doc, if raising a window
	 * overflow/underflow trap is needed regarding the new CWP value. It
	 * seems to be ignored, it would be nice to double check this in real
	 * world though.
	 */

	/* Impl, EC and EF should be ignored */
	/* TODO hardwire version field ? */
	val &= ~((0xf << 28) | (0x3 << 12));
	scpu->reg.psr = val;
	return 0;
}

/**
 * Fetch the specific wim register
 *
 * @param cpu: cpu to fetch WIM from
 * @param val: Register read value
 * @return: Register 0 on read success, -1 otherwise
 */
int scpu_get_wim(struct cpu *cpu, sreg *val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Privilege checking */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	/* Unimplemented windows read as 0 */
	*val = (scpu->reg.wim & ~(0xffffffffULL << (SPARC_NRWIN + 1)));

	return 0;
}

/**
 * Write in specific WIM register
 *
 * @param cpu: cpu to write WIM from
 * @param val: value to write WIM with
 * @return: 0 on success, -1 otherwise
 */
int scpu_set_wim(struct cpu *cpu, sreg val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Privilege checking */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	/* Unimplemented windows read as 0 */
	val &= ~(0xffffffffULL << (SPARC_NRWIN + 1));
	scpu->reg.wim = val;
	return 0;
}

/**
 * Fetch the specific tbr register
 *
 * @param cpu: cpu to fetch TBR from
 * @param val: Register read value
 * @return: Register 0 on read success, -1 otherwise
 */
int scpu_get_tbr(struct cpu *cpu, sreg *val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Privilege checking */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	*val = scpu->reg.tbr;
	return 0;
}

/**
 * Write in specific tbr register
 *
 * @param cpu: cpu to write TBR from
 * @param val: value to write TBR with
 * @return: 0 on success, -1 otherwise
 */
int scpu_set_tbr(struct cpu *cpu, sreg val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* TODO Check supervisor mode */
	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return -1;
	}

	/* TODO Check zero field for debuging purpose ? */

	/* Only TBA is writable */
	val &= (0xfffff << 12);
	scpu->reg.tbr = val;
	return 0;
}

/**
 * Fetch the specific ASR register
 *
 * @param cpu: cpu to fetch ASR from
 * @param asr: ASR number
 * @param val: Register read value
 * @return: Register 0 on read success, -1 otherwise
 */
int scpu_get_asr(struct cpu *cpu, uint8_t asr, sreg *val)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* RDY */
	if(asr == 0)
		*val = scpu->reg.y;
	else if (asr > 15 && asr < 31)
		scpu_tflag_set(cpu, ST_ILL_ISN);

	return 0;
}

/**
 * Write in specific ASR register. This functions takes two values because some
 * ASR stores the xor result of these values some are implementation
 * dependent.
 *
 * @param cpu: cpu to write ASR from
 * @param asr: ASR number
 * @param v1: First value to write ASR with
 * @param v2: Second value to write ASR with
 * @return: 0 on success, -1 otherwise
 */
int scpu_set_asr(struct cpu *cpu, uint8_t asr, sreg v1, sreg v2)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* RDY */
	if(asr == 0)
		scpu->reg.y = v1 ^ v2;
	else if (asr > 15 && asr < 31)
		scpu_tflag_set(cpu, ST_ILL_ISN);

	return 0;
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
 * @param check: set to 1 if check for window overflow is needed
 * @return: 0 on success -1 if a trap occured
 */
static inline int _scpu_window_save(struct cpu *cpu, int check)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint8_t cwp = PSR_CWP(&scpu->reg);

	cwp = ((uint8_t)(cwp - 1)) % SPARC_NRWIN;

	if((check) && (scpu->reg.wim & (1 << cwp))) {
		scpu_tflag_set(cpu, ST_WOVERFLOW);
		return -1;
	}

	PSR_SET_CWP(&scpu->reg, cwp);
	return 0;
}

/**
 * Enter a new register window
 *
 * @param cpu: current cpu
 * @return: 0 on success -1 if a trap occured
 */
void scpu_window_save(struct cpu *cpu)
{
	_scpu_window_save(cpu, 1);
}

/**
 * Exit current register window
 *
 * @param cpu: current cpu
 * @param check: set to 1 if check for window underflow is needed
 * @return: 0 on success -1 if a trap occured
 */
static inline int _scpu_window_restore(struct cpu *cpu, int check)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint8_t cwp = PSR_CWP(&scpu->reg);

	cwp = (cwp + 1) % SPARC_NRWIN;

	/* TODO Trap on window underflow */
	if((check) && (scpu->reg.wim & (1 << cwp))) {
		scpu_tflag_set(cpu, ST_WUNDERFLOW);
		return -1;
	}

	PSR_SET_CWP(&scpu->reg, cwp);
	return 0;
}

/**
 * Exit current register window
 *
 * @param cpu: current cpu
 * @return: 0 on success -1 if a trap occured
 */
void scpu_window_restore(struct cpu *cpu)
{
	_scpu_window_restore(cpu, 1);
}

/**
 * Set cpu trap flag (with perm checking)
 *
 * @param cpu: current cpu
 * @param tn: trap number
 */
void scpu_trap(struct cpu *cpu, uint8_t tn)
{
	/* TODO check permission or trigger privileged trap */
	scpu_tflag_set(cpu, tn);
}

/**
 * Exit current trap
 *
 * @param cpu: current cpu
 * @param jmp: trap return jump address
 */
void scpu_exit_trap(struct cpu *cpu, uint32_t jmp)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	int ret;

	if(!PSR_S(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_PRIV_EXCEP);
		return;
	}

	if(PSR_ET(&scpu->reg)) {
		scpu_tflag_set(cpu, ST_ILL_ISN);
		return;
	}

	ret = _scpu_window_restore(cpu, 1);
	if(ret != 0)
		return;

	/* Instrutions are all 4 byte aligned */
	if(jmp & 0x3) {
		scpu_tflag_set(cpu, ST_MEM_UNALIGNED);
		return;
	}

	scpu_delay_jmp(cpu, jmp);

	/* TODO Change jmpl address space here ? */
	/* XXX Add debug if previous instruction wasn't a jmpl ? */

	PSR_SET_S(&scpu->reg, PSR_PS(&scpu->reg));
	PSR_SET_ET(&scpu->reg, 1);
}

/**
 * Fetch next pipelined instruction
 */
static int scpu_fetch(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret = 0;

	if(scpu_is_error_mode(cpu))
		return -1;

	sreg npc = scpu->reg.pc[1];
	ret = dev_fetch_isn32(cpu->mem, npc, &rd);
	if(ret != 0)
		goto exit;

	scpu->pipeline[1].isn.op = (opcode)be32toh(rd);

exit:
	return ret;
}

/**
 * Decode current pipelined instruction
 */
static int scpu_decode(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	if(scpu_is_error_mode(cpu))
		return -1;

	return isn_decode(&scpu->pipeline[0].isn);
}

/**
 * Boot sparc cpu at specific address
 */
static int scpu_boot(struct cpu *cpu, addr_t addr)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	uint32_t rd;
	int ret;

	/* Initialize PC registers */
	scpu->reg.pc[0] = addr;
	scpu->reg.pc[1] = addr + 4;
	scpu->reg.pc[2] = addr + 8;

	/* Simulate RST trap by enabling Supervisor bit */
	PSR_SET_S(&scpu->reg, 1);

	/* TODO initialize special registers */

	/* Prefetch the first instruction */
	ret = dev_fetch_isn32(cpu->mem, scpu->reg.pc[0], &rd);
	if(ret != 0)
		goto exit;

	scpu->pipeline[0].isn.op = (opcode)be32toh(rd);

	scpu_set_mode(cpu, SM_EXC);
exit:
	return ret;
}

/**
 * Actually handle a trap
 */
static inline int _scpu_enter_trap(struct cpu *cpu, uint8_t tn)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);

	/* Reset CPU on reset trap */
	if(tn == 0) {
		scpu_boot(cpu, 0);
		return 0;
	}

	if(!PSR_ET(&scpu->reg) && !TRAP_IS_INT(tn)) {
		scpu_set_mode(cpu, SM_ERR);
		return 0;
	} else if(!PSR_ET(&scpu->reg)) {
		return 0;
	}

	/* First set proper values for ET, PS and S */
	PSR_SET_ET(&scpu->reg, 0);
	PSR_SET_PS(&scpu->reg, PSR_S(&scpu->reg));
	PSR_SET_S(&scpu->reg, 1);
	/* Then get new register window (without checking for overflow) */
	_scpu_window_save(cpu, 0);
	/* Set %l1,%l2 to PC, nPC XXX what about annul bit ? (see pg 128) */
	scpu_set_reg(cpu, 17, scpu->reg.pc[0]);
	scpu_set_reg(cpu, 18, scpu->reg.pc[1]);
	/* Set TT to proper value */
	TBR_SET_TT(&scpu->reg, tn);
	/* Finally, prepare to jump into trap vector */
	scpu->reg.pc[1] = scpu->reg.tbr;
	scpu->reg.pc[2] = scpu->reg.tbr + 4;
	return scpu_fetch(cpu);
}

/**
 * Execute current pipelined instruction
 */
static int scpu_exec(struct cpu *cpu)
{
	struct sparc_cpu *scpu = to_sparc_cpu(cpu);
	int ret;
	uint8_t tn;

	if(scpu_is_error_mode(cpu))
		return -1;

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

	/* Handle any pending trap */
	if(tq_pending(&scpu->tq, &tn)) {
		ret = _scpu_enter_trap(cpu, tn);
		if(ret != 0)
			return ret;
		tq_ack(&scpu->tq, tn);
	}

	/* Move the pipeline to the next instruction */
	scpu->pipeline[0].isn.op = scpu->pipeline[1].isn.op;
	/* Set next instruction PC registers values */
	scpu->reg.pc[0] = scpu->reg.pc[1];
	scpu->reg.pc[1] = scpu->reg.pc[2];
	scpu->reg.pc[2] += 4;

	return 0;
}

/**
 * Create a sparc cpu instance
 */
static struct cpu *scpu_create(struct cpucfg const *cfg)
{
	struct sparc_cpu *scpu;
	(void)cfg; /* TODO manage sparc families */

	scpu = calloc(1, sizeof(*scpu));
	if(scpu == NULL)
		return NULL;

	return &scpu->cpu;
}

/**
 * Destroy a sparc cpu instance
 */
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
