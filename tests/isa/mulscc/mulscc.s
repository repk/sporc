.section .text, "ax", @progbits

tmain:
	/* 0x6f8f2 * 0x1fefdd4d = 0xdeadbeefcaca */
	sethi %hi(0x1fefdd4d), %g1
	wr %g1, %lo(0x1f3fdd4d), %y
	sethi %hi(0x6f8f2), %g1
	or %g1, %lo(0x6f8f2), %g1
	orcc %g0, %g0, %g2 /* Zero partial product and clear N and V */
	/* 33 iterations of mulscc */
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g1, %g2
	mulscc %g2, %g0, %g2
	rd %y, %g1
