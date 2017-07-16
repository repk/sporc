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
