.section .text, "ax", @progbits

.align 4096

.macro TRAP_RESET
	call tmain
	nop;nop;nop
.endm

.macro TRAP_EMPTY
1:
	ba 1b
	nop
	nop
	nop
.endm

.macro TRAP_OK
	sethi %hi(0xdeadbeef), %g2
	or %g2, %lo(0xdeadbeef), %g2
	jmpl %l2, %g0
	rett %l2 + 4
.endm

/* Define Trap vector */
TRAP_RESET; TRAP_EMPTY; TRAP_OK; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
tmain:
	/* Enable trap */
	rd %psr, %g1
	wr %g1, 0x20, %psr
	nop; nop; nop

	rd %y, %g1
	or %g0, 17, %g2
	rd %asr21, %g2
1:
	ba 1b
	nop
