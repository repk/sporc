.section .text, "ax", @progbits

tmain:
	/* 0xcafe * 0xdeadbeef = 0xffffe594090c0722 */
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	sethi %hi(0xcafe), %g2
	or %g2, %lo(0xcafe), %g2
	smul %g1, %g2, %g1
	rd %y, %g2
