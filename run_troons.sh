#!/bin/bash

set -x
set -e # Exit if error

if [[ $# -ne 3 ]]; then
    echo "Wrong number of arguments"
    exit 2
fi

EXEC=$1
INPUT=$2
ARCH=$3

cp cs3210-a1-a1-a0200705x_a0260770h/$EXEC cs3210-a1-a1-a0200705x_a0260770h/testcases/$INPUT /home/$USER
cp /home/$USER/$EXEC /home/$USER/$INPUT /nfs/home/$USER

export EXEC
export INPUT

jobid=$(sbatch --partition $ARCH --parsable ./troons.sh)

ln -nsf /nfs/home/$USER/$EXEC-$jobid.log ./$EXEC-$ARCH-latest.log
# cp /nfs/home/$USER/perf.data ./perf.data
