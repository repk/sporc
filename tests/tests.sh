#!/bin/sh

test() {
	echo -n "Test ${1}:\t\t"
	$(dirname ${0})/t-${1}
}

test sethi
test or
test orcc
test orn
test orncc
test and
test andcc
test andn
test andncc
test xor
test xorcc
test xnor
test xnorcc
test add
test addcc
test sub
test subcc
test stb
test sth
test st
test std
test ldsb
test ldub
test ldsh
test lduh
test ld
test ldd
test call
test jmpl
test ba
test bn
test bne
test be
test bg
test ble
test bge
test bl
test bgu
test bleu
test bcc
test bcs
test bpos
test bneg
test bvc
