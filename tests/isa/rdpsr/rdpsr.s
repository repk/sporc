.section .text, "ax", @progbits

tmain:
	restore %g0, %g0, %g0 /* increment cwp */
	subcc %g0, %g0, %g0 /* Set zero flag */
	rd %psr, %g1
