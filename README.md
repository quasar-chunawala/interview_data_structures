# Interview Data Structures
This repository contains homegrown versions of various important data-structures used in quantitative trading and finance. My goal is to develop an intuition for the kind of code required to write these types and not replace the standard facilities - they exist, they work, they are tested ad nauseam. 

## Installation

```shell
git clone --recurse-submodules -j8 https://github.com/quasar-chunawala/interview_data_structures.git 
```

## Build

Run `cmake` to generate the build system:

```shell
mkdir build
cd build
cmake ..
```

Build the project by issuing:

```shell
cmake --build .
```

Run all tests by issuing :

```shell
ctest
```
