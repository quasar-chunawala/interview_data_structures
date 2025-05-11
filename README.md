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

## Generating code coverage reports

Ensure that `gcov`, `lcov` and `genhtml` are installed.

Check if the `gcov` version matches the `compiler` version using the build the code:

```shell
gcov --version
gcc --version
```

On Arch Linux, install `lcov` using `pacman`:

```shell
sudo pacman -S lcov
```

On Ubuntu/Debian, install `lcov` using `apt`:

```shell
sudo apt update
sudo apt install lcov
```

On Fedora, install `lcov` using `dnf`:

```shell
sudo dnf install lcov
```

On macOS, install `lcov` using `homebrew`:

```shell
brew install lcov
```

Once all the dependencies are installed, from the root folder, run the following set of commands:

```shell
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run the tests
ctest

# Generate the coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.filtered.info
genhtml coverage.filtered.info --output-directory coverage_report
```