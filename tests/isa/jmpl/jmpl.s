.section .text, "ax", @progbits

tmain:
	nop
	jmpl %lo(func1), %g2  /* PC is 0x4 here */
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0x00000fff), %g1 /* Hopefully a dead spot */

.align 32
func1: /* This is at @0x20 */
	or %g1, %lo(0xdeadbeef), %g1
	or %g0, %lo(func2), %g3
	jmpl %g3, %g4 /* PC is 0x28 */
	sethi %hi(0xb16b00b5), %g5
	or %g5, %lo(0x00000fff), %g5 /* Hopefully a dead spot */

.align 32
func2:
	or %g5, %lo(0xb16b00b5), %g5
