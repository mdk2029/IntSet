### Kset

A simplified `BTree` like class that provides an ordered collection of 64 bit ints by packing 6 ints into each node.

Please see inline documentation in [kset/kset_node.h] for design details

### Building

Only linux is supported. Check out the [CMakeLists.txt] file for choosing compilers and enabling explicit usage of SIMD instructions `(AVX2)` when finding in a node. Uses `googletest` for unit tests and `google-benchmark` for benchmarks (compared against `std::set<int64_t>`)

### Running

Build the project and run `test/intset_test --help` and `bench/intset-bench --help`

### Benchmark results

Look in `bench` subdir for the benchmark functions. For size `N`, we repeat an operation [lookup/successor] `N` times.
Here are the results on my laptop (Intel i5 3.2GHz Quadcore)

| Operation | Num elements (millions) | Set time (secs)| Kset time (secs)|
|:---------:|:-----------------------:|:--------------:|:---------------:|
| Lookup    | 4M                      |  5.26s         | 2.14s           |
| Lookup    | 8M                      |  12.17s        | 4.80s           |
| Lookup    | 16M                     |  29.04s        | 11.0s           |
| Lookup    | 32M                     |  67.71s        | 23.38s          |
| Successor | 4M                      |  7.59s         | 2.2s            |
| Successor | 8M                      |  16.67s        | 5.0s            |
| Successor | 16M                     |  36.81s        | 11.2s           |
| Successor | 32M                     |  75.31s        | 24.0s           |


