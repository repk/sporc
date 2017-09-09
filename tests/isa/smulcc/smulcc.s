.section .text, "ax", @progbits

tmain:
	/* 0xb16cafe * 0xdeadbeef = 0xfe8e7fb6b6960722 */
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	sethi %hi(0xb16cafe), %g2
	or %g2, %lo(0xb16cafe), %g2
	smulcc %g1, %g2, %g1
	bpos,a deadloop /* N flag should be set here */
	or %g0, %g0, %g2
	rd %y, %g2

	/* 0x80000000 * 0xb16cafe0 = 0xfa749a8100000000 */
	sethi %hi(0x80000000), %g1
	or %g1, %lo(0x80000000), %g1
	sethi %hi(0xb16cafe), %g2
	or %g2, %lo(0xb16cafe), %g2
	smulcc %g1, %g2, %g1
	bne,a deadloop /* N flag should be set here */
	or %g0, %g0, %g2
	rd %y, %g2

deadloop:
	ba deadloop
	nop
