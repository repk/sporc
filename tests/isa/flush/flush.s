.section .text, "ax", @progbits

tmain:
	sethi %hi(0x0337ab6f), %g1
	or %g1, %lo(0x0337ab6f), %g1
	st %g1, [%g0 + 0x10]
	flush %g0 + 0x10
	nop
	or %g1, %lo(0xdeadbeef), %g1
