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
1:
	ba 1b

tmain:
	/* Do not trap */
	addcc %g0, 0x1fff, %g1
	tcs 5

	/* Do trap */
	addcc %g1, 1, %g0
	tcs 4
1:
	ba 1b