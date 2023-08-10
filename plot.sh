#!/bin/bash

TOPDIR=$(readlink -f $(dirname $0))
PYTHON=python3

if [ "$1" == "paper" ]; then
	CSV_RAND="$TOPDIR/csv_from_paper/results_rand.csv"
	CSV_1T="$TOPDIR/csv_from_paper/results_1t.csv"
else
	CSV_RAND="$TOPDIR/results_rand.csv"
	CSV_1T="$TOPDIR/results_1t.csv"
fi

cd $TOPDIR

if [ -e $CSV_RAND ]; then
	echo "##################################################"
	echo "Plotting 'rand' results"
	echo "##################################################"
	$PYTHON plots/plot-rand.py $CSV_RAND
fi

if [ -e $CSV_1T ]; then
	echo "##################################################"
	echo "Plotting '1t' results"
	echo "##################################################"
	$PYTHON plots/plot-1t.py $CSV_1T
fi

