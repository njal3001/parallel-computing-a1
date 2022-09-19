#!/bin/bash

set -x
set -e # Exit if error

if [[ $# -ne 2 ]]; then
    echo "Wrong number of arguments"
    exit 2
fi

EXEC=$1
INPUT=$2
#ARCH="xs-4114"
ARCH="i7-7700"

cp cs3210-a1-a1-a0200705x_a0260770h/$EXEC cs3210-a1-a1-a0200705x_a0260770h/testcases/$INPUT /home/$USER
cp /home/$USER/$EXEC /home/$USER/$INPUT /nfs/home/$USER

export EXEC
export INPUT

jobid=$(sbatch --partition $ARCH --parsable --wait ./troons.sh)

cat /nfs/home/$USER/troons_$jobid.log
