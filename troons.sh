#!/bin/bash
#SBATCH --job-name=troons
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --mem=1gb
#SBATCH --time=00:00:30
#SBATCH --output=troons_%j.log
#SBATCH --error=troons_%j.log

echo "Running Troons Job!"
echo "Running on $(hostname)"
echo "Executing $EXEC with input $INPUT"

rm /home/$USER/$EXEC
rm /home/$USER/$INPUT
sbcast /nfs/home/$USER/$EXEC /home/$USER/$EXEC
sbcast /nfs/home/$USER/$INPUT /home/$USER/$INPUT

echo ""
echo "Output:"
echo ""

perf stat -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations,duration_time /home/$USER/$EXEC $INPUT

cp "troons_$SLURM_JOB_ID.log" "/nfs/home/$USER/"
