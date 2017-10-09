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
TRAP_RESET; TRAP_EMPTY; TRAP_TEST;

trapjmp:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
1:
	ba 1b
	nop

tmain:
	/* Enable trap */
	rd %psr, %g1
	wr %g1, 0x20, %psr
	nop; nop; nop

	unimp 0x12
	/* Hopefully this is a deadspot */
	sethi %hi(0xb16b00b5), %g1
	or %g1, %lo(0xb16b00b5), %g1
1:
	ba 1b
	nop
