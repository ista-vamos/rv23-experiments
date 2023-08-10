# rv23-experiments
Experiments from the paper "Monitoring Hyperproperties With Prefix Transducers" accepted at RV'23

## Building

### Requirements

### Configuration & Build

```shell
# clone the repo
git checkout https://github.com/ista-vamos/rv23-experiments
cd rv23-experiments

# initialize and build vamos-buffers
git submodule update --init -- vamos-buffers
cd vamos-buffers
cmake . -DCMAKE_BUILD_TYPE=Release
make -j4
cd -

# configure and build the project
cmake .
make -j4
```

## Running

In the top level directory is the script `run.sh` that takes as an argument either
`rand` to run experiments on random traces, or `1t` to run experiments on
multiple instances of the same trace. So run the experiments as follows:

```
./run.sh rand
./run.sh 1t
```

By default, the script repeats the experiments 10 times (can be modified by changing
the `REP` variable in the script). If you run the script multiple times, it will
prompt the user if the old results should be overwritten or not.

### Customizing

As stated above, the number of repetitions of the experiments can be set by the
`REP` variable in the script `run.sh`. There is also the variable `NPROC`
that controls how many processes should be run in parallel.
By default, it is empty which means that the script should detect the number of cores
and use a suitable number of processes. You can set this variable to a number
of processes that you want to use.
