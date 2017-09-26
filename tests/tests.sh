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
test isa sll
test isa srl
test isa sra
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
test isa ldsba
test isa ldub
test isa lduba
test isa ldsh
test isa ldsha
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
test isa rdpsr
test isa wrpsr
test isa ta
test isa tn
test isa tne
test isa te
test isa tg
test isa tle
test isa tge
test isa tl
test isa tgu
test isa tleu
test isa tcc
test isa tcs
test isa tpos
test isa tneg
test isa tvc
test isa tvs
test isa wrwim
test isa rdwim
test isa wrtbr
test isa rdtbr
test isa rdasr
test isa wrasr
test isa mulscc
test isa umul
test isa umulcc
test isa smul
test isa smulcc
test isa udiv
test isa udivcc
test isa sdiv
test isa sdivcc

printf "${RES}" | column -t

if [ ${ERR} -eq 1 ]; then
	exit 255
fi
