.section .text, "ax", @progbits

.align 4096

.macro TRAP_EMPTY
1:
	ba 1b
	nop
	nop
	nop
.endm

.macro TRAP_RESET
	call tmain
	nop;nop;nop
.endm

.macro TRAP_JUMP
	call trapjmp
	nop;nop;nop
.endm

/* Define Trap vector */
TRAP_RESET; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY; TRAP_EMPTY
TRAP_EMPTY; TRAP_EMPTY; TRAP_JUMP;  TRAP_EMPTY

trapjmp:
	andcc %g1, 0x3, %g1
	bnz ok
	sethi %hi(0xfee1dead), %g1
	ba end
	or %g1, %lo(0xfee1dead), %g1
ok:
	sethi %hi(0xdeadbeef), %g1
	ba end
	or %g1, %lo(0xdeadbeef), %g1

tmain:
	/* Enable trap */
	rd %psr, %g1
	wr %g1, 0x20, %psr
	nop; nop; nop

	/* Do not trap */
	sethi %hi(0x21524114), %g1
	or %g1, %lo(0x21524114), %g1
	taddcctv %g1, %g1, %g1

	/* Trap */
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	taddcctv %g1, %g1, %g1

	/* Hopefully this is a deadspot */
	sethi %hi(0xb16b00b5), %g1
	or %g1, %lo(0xb16b00b5), %g1
end:
	ba end
	nop

