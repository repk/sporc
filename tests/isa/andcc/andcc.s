.section .text, "ax", @progbits

tmain:
	sethi %hi(0xcacacaca), %g1
	or %g1, %lo(0xcacacaca), %g1
	sethi %hi(0x8000ffff), %g2
	or %g2, %lo(0x8000ffff), %g2
	andcc %g1, %g2, %g1
	sethi %hi(0x00caca00), %g1
	or %g1, %lo(0x00caca00), %g1
	sethi %hi(0xff0000ff), %g2
	or %g2, %lo(0xff0000ff), %g2
	andcc %g1, %g2, %g1
