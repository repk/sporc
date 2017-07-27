.section .text, "ax", @progbits

tmain:
	sethi %hi(0xbeef), %g1
	or %g1, %lo(0xbeef), %g1
	sth %g1, [%g0 + 0x10]
