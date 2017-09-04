#!/bin/sh

RES=""
ERR=0

test() {
	RES="${RES} Test ${2}:\t\t $($(dirname ${0})/${1}/t-${2} 2>&1)\n"
	if [ ${?} -ne 0 ]; then
		ERR=1
	fi
}

test isa sethi
test isa or
test isa orcc
test isa orn
test isa orncc
test isa and
test isa andcc
test isa andn
test isa andncc
test isa xor
test isa xorcc
test isa xnor
test isa xnorcc
test isa add
test isa addcc
test isa sub
test isa subcc
test isa g0
test isa stb
test isa sth
test isa st
test isa std
test isa ldsb
test isa ldub
test isa ldsh
test isa lduh
test isa ld
test isa ldd
test isa call
test isa jmpl
test isa ba
test isa bn
test isa bne
test isa be
test isa bg
test isa ble
test isa bge
test isa bl
test isa bgu
test isa bleu
test isa bcc
test isa bcs
test isa bpos
test isa bneg
test isa bvc
test isa bvs
test isa save-restore
test isa ta
test isa tn
test isa tne
test isa te
test isa rdpsr
test isa wrpsr
test isa wrwim
test isa rdwim
test isa wrtbr
test isa rdtbr

printf "${RES}" | column -t

if [ ${ERR} -eq 1 ]; then
	exit 255
fi
