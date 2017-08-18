#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "trap.h"

#define SP_TISN(n) (0x20 + (n))
#define SP_TINT(n) (0xaf - (n))

/**
 * Convert trap priority index into trap number
 */
static inline uint8_t _prio_to_tn(uint8_t prio)
{
	uint8_t ret = 0;

	static uint8_t const ptrap[] = {
		ST_RST,
		ST_DST_ERR,
		ST_IACCESS_MMU_MISS,
		ST_IACCESS_ERR,
		ST_RREG_ACCESS_ERR,
		ST_IACCESS_EXCEP,
		ST_PRIV_EXCEP,
		ST_ILL_ISN,
		ST_FP_DISABLE,
		ST_CP_DISABLE,
		ST_UNIMPL_FLUSH,
		ST_WATCHPOINT_DETECT,
		ST_WOVERFLOW,
		ST_WUNDERFLOW,
		ST_MEM_UNALIGNED,
		ST_FP_EXCEP,
		ST_CP_EXCEP,
		ST_DACCESS_ERR,
		ST_DACCESS_MMU_MISS,
		ST_DACCESS_EXCEP,
		ST_TAG_OVERFLOW,
		ST_DIV_BY_ZERO,
	};

	if(prio < ARRAY_SIZE(ptrap))
		ret = ptrap[prio];
	else if(prio <= SP_TISN(0x7f))
		ret = ST_TISN(prio - SP_TISN(0));
	else if(prio <= SP_TINT(0))
		ret = ST_TINT(prio - SP_TINT(15));

	return ret;
}

/**
 * Convert trap number into trap priority index
 */
static inline uint8_t _tn_to_prio(uint8_t tn)
{
	uint8_t ret = 0;

	static uint8_t const tprio[] = {
		[ST_RST] = 0,
		[ST_DST_ERR] = 1,
		[ST_IACCESS_MMU_MISS] = 2,
		[ST_IACCESS_ERR] = 3,
		[ST_RREG_ACCESS_ERR] = 4,
		[ST_IACCESS_EXCEP] = 5,
		[ST_PRIV_EXCEP] = 6,
		[ST_ILL_ISN] = 7,
		[ST_FP_DISABLE] = 8,
		[ST_CP_DISABLE] = 9,
		[ST_UNIMPL_FLUSH] = 10,
		[ST_WATCHPOINT_DETECT] = 11,
		[ST_WOVERFLOW] = 12,
		[ST_WUNDERFLOW] = 13,
		[ST_MEM_UNALIGNED] = 14,
		[ST_FP_EXCEP] = 15,
		[ST_CP_EXCEP] = 16,
		[ST_DACCESS_ERR] = 17,
		[ST_DACCESS_MMU_MISS] = 18,
		[ST_DACCESS_EXCEP] = 19,
		[ST_TAG_OVERFLOW] = 20,
		[ST_DIV_BY_ZERO] = 21,
	};

	if(TRAP_IS_INT(tn))
		ret = SP_TINT(tn - ST_TINT(0));
	else if(TRAP_IS_ISN(tn))
		ret = SP_TISN(tn - ST_TISN(0));
	else if(tn < ARRAY_SIZE(tprio))
		ret = tprio[tn];

	return ret;
}

/**
 * Add outstanding trap in trap queue
 *
 * @param tq: The trap queue
 * @param tn: The trap number
 */
void tq_raise(struct trap_queue *tq, uint8_t tn)
{
	uint8_t idx = _tn_to_prio(tn);

	tq->tflag[idx >> 3] |= 1 << (idx % 8);
}

/**
 * Remove a received/ack'ed trap from the trap queue
 *
 * @param tq: The trap queue
 * @param tn: The trap number
 */
void tq_ack(struct trap_queue *tq, uint8_t tn)
{
	uint8_t idx = _tn_to_prio(tn);

	tq->tflag[idx >> 3] &= ~(1 << (idx % 8));
}

/**
 * Yeld the prending trap with highest priority
 *
 * @param tq: The trap queue
 * @param tn: Set to the highest priority pending trap number
 *
 * @return: 0 if no trap are pending, 1 otherwise
 */
int tq_pending(struct trap_queue *tq, uint8_t *tn)
{
	size_t i;
	int ret = 0, idx;

	for(i = 0; i < ARRAY_SIZE(tq->tflag); ++i) {
		idx = ffs(tq->tflag[i]);
		if(!idx)
			continue;
		*tn = _prio_to_tn((i * 8) + idx - 1);
		ret = 1;
		break;
	}

	return ret;
}
