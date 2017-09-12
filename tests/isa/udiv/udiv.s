.section .text, "ax", @progbits

tmain:
	/* 0xb092090c0722 / 0xcafe = 0xdeadbeef */
	sethi %hi(0xb092), %g1
	wr %g1, %lo(0xb092), %y
	sethi %hi(0x090c0722), %g1
	or %g1, %lo(0x090c0722), %g1
	sethi %hi(0xcafe), %g2
	or %g2, %lo(0xcafe), %g2
	udiv %g1, %g2, %g3
	or %g0, %lo(0xfe), %g2
	udiv %g1, %g2, %g4
