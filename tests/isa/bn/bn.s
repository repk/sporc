.section .text, "ax", @progbits

tmain:
	bn 1f
	sethi %hi(0xdeadbeef), %g1
	bn,a 1f
	sethi %hi(0xcacacaca), %g1
	or %g1, %lo(0xdeadbeef), %g1
1:
	sethi %hi(0xd00d0000), %g1
