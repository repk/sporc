.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g2
	or %g2, %lo(0xdeadbeef), %g2
	sethi %hi(0x21524114), %g3
	or %g3, %lo(0x21524114), %g3
	taddcc %g3, %g3, %g3
	bvs oups
	taddcc %g2, %g2, %g2
	bvc oups
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1

oups:
	sethi %hi(0xfee1dead), %g1
	or %g1, %lo(0xfee1dead), %g1
1:
	ba 1b
	nop
