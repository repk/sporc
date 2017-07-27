#!/bin/sh

RES=""

test() {
	RES="${RES} Test ${1}:\t\t $($(dirname ${0})/t-${1} 2>&1)\n"
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
test bvs
test save-restore

printf "${RES}" | column -t
