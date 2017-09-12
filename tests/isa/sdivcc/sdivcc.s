.section .text, "ax", @progbits

tmain:
	/* 0xffff4f6df6f3f8de / 0xffff3502 = 0xdeadbeef -> no overflow */
	sethi %hi(0xffff4f6d), %g1
	wr %g1, %lo(0xffff4f6d), %y
	sethi %hi(0xf6f3f8de), %g1
	or %g1, %lo(0xf6f3f8de), %g1
	sethi %hi(0xffff3502), %g2
	or %g2, %lo(0xffff3502), %g2
	sdivcc %g1, %g2, %g3
	bvs 1f
	/* 0xffff4f6df6f3f8de / 0xfe = 0xffffff4e0a0b0a0d -> negative overflow */
	or %g0, %lo(0xfe), %g2
	sdivcc %g1, %g2, %g5
	bvc 1f
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
1:
	ba 1b
	nop
