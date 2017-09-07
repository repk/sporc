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
	sethi %hi(0xdeadbeef), %g4
	or %g4, %lo(0xdeadbeef), %g4
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

	sethi %hi(0xdeadbeef), %g1
	wr %g1, %lo(0xdeadbeef), %y
	nop; nop; nop
	rd %y, %g2
	sethi %hi(0xdeaddead), %g1
	wr %g1, %lo(0xdeaddead), %asr0 /* Should be synonym for wry */
	nop; nop; nop
	rd %y, %g3

	or %g0, 17, %g4
	wr %g0, 0, %asr21
	nop; nop; nop

1:
	ba 1b
	nop
