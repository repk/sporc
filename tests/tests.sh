#!/bin/sh

test() {
	echo -n "Test ${1}:\t\t"
	$(dirname ${0})/t-${1}
}

test sethi
test or
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
test and
test xor
test andn
test orn
test xnor
test orcc
test orncc
test andcc
test andncc
test xorcc
test xnorcc
