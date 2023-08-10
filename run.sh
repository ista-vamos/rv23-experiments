#!/bin/bash

set -e

#################################################
# How many times to repeat the experiments
REP=10
# How many processes run in parallel
NPROC=
# Python interpreter
PYTHON=python3
#################################################

if [ "$1" != "rand" -a "$1" != "1t" ]; then
	echo "Usage: $0 [rand | 1t]"
	echo "'rand' means run on random traces, '1t' are experiments with instances of a single trace"
	exit 1
fi

TOPDIR=$(readlink -f $(dirname $0))
RESDIR=$TOPDIR/results
mkdir -p $RESDIR
OUT="$RESDIR/results_$1.csv"

echo "Output file: $OUT"

cd $TOPDIR/mpt-monitors

if [ -e $OUT ]; then
	echo "The output file $OUT exists."
	echo -e "\033[0;34mShould I delete it? (If no, I will append results)[y/n]\033[0m"
	read Q
	while [ "$Q" != "y" -a "$Q" != "n" ]; do
		echo "Please type 'y' or 'n']"
		read Q
	done

	if [ "$Q" == "y" ]; then
		echo "Overwriting $OUT"
		rm $OUT
		touch $OUT
	else
		echo "Appending to $OUT"
	fi
fi

echo "---------------------------------------------"
echo "Starting experiments, doing $REP repetitions."
echo "---------------------------------------------"
for i in $(seq 1 $REP); do
	$PYTHON ./run.py "monitor_$1" $NPROC | tee --append $OUT
done
echo "---------------------------------------------"
echo "Done."
echo "---------------------------------------------"
