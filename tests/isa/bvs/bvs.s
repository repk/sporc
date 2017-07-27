.section .text, "ax", @progbits

tmain:
	/* Do not branch do not annul */
	subcc %g0, %g0, %g0
	bvs 1f
	sethi %hi(0xdeadbeef), %g1


	/* Do not branch does annul */
	addcc %g0, %g1, %g1
	bvs,a 1f
	sethi %hi(0xcacacaca), %g1

	/* Do branch + delay slot */
	sethi %hi(0x80000000), %g2
	subcc %g2, 0x1, %g2
	bvs 2f
	or %g1, %lo(0x0000be00), %g1
2:
	or %g1, %lo(0x000000ef), %g1

1:
	sethi %hi(0xd00d0000), %g1
