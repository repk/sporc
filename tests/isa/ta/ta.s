.section .text, "ax", @progbits

.align 4096

.macro TRAP_RESET
	call tmain
	nop;nop;nop
.endm

.macro TRAP_EMPTY
	nop;nop;nop;nop
.endm

.macro TRAP_TEST
	call trapjmp
	nop;nop;nop
.endm

/* Define Trap vector */
TRAP_RESET; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY

TRAP_TEST /* Test trap number 132 (ta 4) */

trapjmp:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	or %g0, %l1, %g2

tmain:
	/* Enable trap */
	rd %psr, %g1
	wr %g1, 0x20, %psr
	nop; nop; nop

	or %g0, %g0, %g1
	ta 4
	/* Hopefully this is a deadspot */
	sethi %hi(0xb16b00b5), %g1
	or %g1, %lo(0xb16b00b5), %g1
