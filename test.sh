#!/bin/bash

function run() {
	echo -n "$1 ... "
	result=$(./bin/lisp "$1")
	if [ "$result" != "$2" ]; then
		echo FAILED
		exit 1
	fi
	echo ok
}

run test/bool.scm "#f"
run test/bool_real1.scm "#f"
run test/bool_real2.scm "#t"
run test/EuclideanAlgorithm.scm "31"
run test/comment.scm "3"
run test/cul.scm "-544"
run test/define.scm "32"
run test/define2.scm "12"
run test/fact.scm "3628800"
run test/fibonacci.scm "55"
run test/fibonacci2.scm "55"
run test/func.scm "3"
run test/if.scm "0"
run test/lambda.scm "6"
run test/lambda2.scm "18"
run test/mod.scm "0"
run test/name.scm "6"
run test/name_adv.scm "10"
run test/neg.scm "98"
run test/print.scm "1"
run test/real_plus.scm "8.000000"
run test/realtype.scm "3.141535"
run test/real_minus.scm "-0.700000"
run test/real_time.scm "1.860000"
run test/real_dev.scm "1.500000"
run test/recursion.scm "7"

echo "All tests were done."
