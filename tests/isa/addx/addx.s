.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeee), %g1
	or %g1, %lo(0xdeadbeee), %g1
	addcc %g1, %g1, %g2
	addx %g0, %g1, %g1
