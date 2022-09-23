#!/bin/bash

set -x
set -e # Exit if error

if [[ $# -ne 1 ]]; then
    echo "Wrong number of arguments"
    exit 2
fi

TESTCASE=$1

./troons testcases/$TESTCASE.in > testcases/$TESTCASE.out
./troons_seq2 testcases/$TESTCASE.in > testcases/$TESTCASE.output
diff testcases/$TESTCASE.out testcases/$TESTCASE.output