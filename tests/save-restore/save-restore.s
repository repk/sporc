.section .text, "ax", @progbits

tmain:
	sethi %hi(0xb16b00b5), %o2
	or %o2, %lo(0xb16b00b5), %o2
	or %g0, 0xfc, %sp
	save %sp, -0xc, %sp
	sethi %hi(0xdeadbeef), %o2
	or %o2, %lo(0xdeadbeef), %o2
	restore %g0, %g0, %g0
