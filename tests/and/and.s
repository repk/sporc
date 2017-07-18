.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	and %g1, 0xfff, %g1
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	sethi %hi(0xfffff000), %g2
	or %g2, %lo(0xfffff000), %g2
	and %g1, %g2, %g1
