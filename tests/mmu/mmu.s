.section .text, "ax", @progbits

/*
CTXTBL: 0x400 -> 1024
LVL1 :  0x800 -> 1024
LVL2 :  0x900 ->  256 (We overlap LVL1 table, but should be fine on low addr)
LVL3 :  0xa00 ->  256

VA: CTX:0, 0x01083100 (LVL1:0x1, LVL2:0x2, LVL3:0x3, Offset 0x100) -> PA: 0x100

Also because code is on the first physical page, it is being mapped in the first
virtual page so that enabling MMU goes smoothly.
*/

tmain:
	/* First place a beacon at 0x100 in physical memory */
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	st %g1, [%g0 + 0x100]

	/* Set Context Number */
	or %g0, 0x200, %g1
	sta %g0, [%g1] 0x4

	/* Set Context Table address */
	or %g0, 0x100, %g2
	or %g0, 0x40, %g1 /* 0x40: (0x400 >> 6) << 2 */
	sta %g1, [%g2] 0x4

	/* Set Context's LVL1 PA */
	or %g0, 0x81, %g1 /* Ox81: (0x800 >> 6) << 2 | ET == PTD */
	st %g1, [%g0 + 0x400]

	/* Set LVL1's LVL2 PA */
	or %g0, 0x91, %g1 /* Ox25: (0x900 >> 6) << 2 | ET == PTD */
	st %g1, [%g0 + 0x804]
	st %g1, [%g0 + 0x800]

	/* Set LVL2's LVL3 PA */
	or %g0, 0xa1, %g1 /* Oxa9: (0xa00 >> 6) << 2 | ET == PTD */
	st %g1, [%g0 + 0x908]
	st %g1, [%g0 + 0x900]

	/* Set LVL3's page PA */
	or %g0, 0x0e, %g1 /* 0x0e: (0x0 >> 6) << 8 | ACC == RWX | ET == PTE */
	st %g1, [%g0 + 0xa0c]
	st %g1, [%g0 + 0xa00]

	lda [%g0] 0x4, %g1
	or %g1, 0x1, %g1
	sta %g1, [%g0] 0x4 /* Enable MMU */

	sethi %hi(0x01083100), %g1
	or %g1, %lo(0x01083100), %g1
	ld [%g1], %g1 /* Get our beacon back */

	/* Try with LVL1 cache hit */
	sethi %hi(0x01003100), %g2
	or %g2, %lo(0x01003100), %g2
	ld [%g2], %g2 /* Get our beacon back */
