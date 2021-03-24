# Descartes
A Pascal compiler written in C++.
## Build
Descartes uses the Conan package manager to fetch its dependencies.
```
$ mkdir build/ && cd build/
$ conan install ..
$ cmake -G Ninja ..
$ ninja
```
## Usage
To run the compiler.
```
$ ./bin/descartes [OPTIONS] file
```
To run the unit tests.
```
$ ./bin/descartes_test
```
