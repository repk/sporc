.section .text, "ax", @progbits

tmain:
	nop
	call func /* PC is 0x4 here */
	sethi %hi(0xdeadbeef), %g1

.align 32
func:
	or %g1, %lo(0xdeadbeef), %g1
