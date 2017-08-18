#ifndef _TRAP_H_
#define _TRAP_H_

#define ST_RST 0x00
#define ST_DST_ERR 0x2b
#define ST_IACCESS_MMU_MISS 0x3c
#define ST_IACCESS_ERR 0x21
#define ST_RREG_ACCESS_ERR 0x20
#define ST_IACCESS_EXCEP 0x01
#define ST_PRIV_EXCEP 0x03
#define ST_ILL_ISN 0x02
#define ST_FP_DISABLE 0x04
#define ST_CP_DISABLE 0x24
#define ST_UNIMPL_FLUSH 0x25
#define ST_WATCHPOINT_DETECT 0x0b
#define ST_WOVERFLOW 0x05
#define ST_WUNDERFLOW 0x06
#define ST_MEM_UNALIGNED 0x07
#define ST_FP_EXCEP 0x08
#define ST_CP_EXCEP 0x28
#define ST_DACCESS_ERR 0x29
#define ST_DACCESS_MMU_MISS 0x2c
#define ST_DACCESS_EXCEP 0x09
#define ST_TAG_OVERFLOW 0x0a
#define ST_DIV_BY_ZERO 0x2a
#define ST_TISN_MIN 0x80
#define ST_TISN_MAX 0xff
#define ST_TISN(n) (ST_TISN_MIN + (n))
#define ST_TINT_MAX 0x1f
#define ST_TINT_MIN 0x11
#define ST_TINT(n) (ST_TINT_MIN + (n))

/* Trap raised by an external interrupt */
#define TRAP_IS_INT(tn) (((tn) >= ST_TINT_MIN) && ((tn) <= ST_TINT_MAX))
/* Trap raised by an instruction exception */
#define TRAP_IS_ISN(tn) ((tn) >= ST_TISN_MIN)

struct trap_queue {
	/* Trap flags */
	uint8_t tflag[32];
};

void tq_raise(struct trap_queue *tq, uint8_t tn);
void tq_ack(struct trap_queue *tq, uint8_t tn);
int tq_pending(struct trap_queue *tq, uint8_t *tn);
int trap_is_interrupt(uint8_t tn);

#endif
