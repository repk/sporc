.section .text, "ax", @progbits

.align 4096

.macro TRAP_RESET
	call tmain
	nop;nop;nop
.endm

.macro TRAP_EMPTY
	nop;nop;nop;nop
.endm

.macro TRAP_WUF
	call wufjmp
	nop;nop;nop
.endm

/* Define Trap vector */
TRAP_RESET; TRAP_EMPTY; TRAP_EMPTY;
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY;

TRAP_WUF;

wufjmp:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1

tmain:
	/* Enable trap */
	rd %psr, %g1
	wr %g1, 0x20, %psr
	nop; nop; nop

	or %g0, 0, %g1 /* Clear g1 */
	wr %g0, 4, %wim /* Make window 2 invalid */
	nop; nop; nop;
	restore /* Should be safe here */
	restore /* Cause window underflow trap here */
	/* Hopefully this is a deadspot */
	sethi %hi(0xb16b00b5), %g1
	or %g1, %lo(0xb16b00b5), %g1
