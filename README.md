# rv23-experiments
Experiments from the paper "Monitoring Hyperproperties With Prefix Transducers" accepted at RV'23

## Build

```
git checkout https://github.com/ista-vamos/rv23-experiments
cd rv23-experiments

# initialize vamos-buffers
git submodule update --init -- vamos-buffers

# configure and build the project
cmake .
make -j4
```
