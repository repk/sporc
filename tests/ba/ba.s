.section .text, "ax", @progbits

tmain:
	ba 1f
	sethi %hi(0xdeadbeef), %g1
	sethi %hi(0xcacacaca), %g1
1:
	ba,a 2f
	or %g1, %lo(0x0000caca), %g1
	sethi %hi(0xd00d0000), %g1
2:
	or %g1, %lo(0xdeadbeef), %g1
