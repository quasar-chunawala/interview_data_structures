#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install lcov based on the operating system
install_lcov() {
    echo "Installing lcov..."
    if command_exists pacman; then
        echo "Detected Arch Linux. Installing lcov using pacman..."
        sudo pacman -S lcov
    elif command_exists apt; then
        echo "Detected Ubuntu/Debian. Installing lcov using apt..."
        sudo apt update
        sudo apt install -y lcov
    elif command_exists dnf; then
        echo "Detected Fedora. Installing lcov using dnf..."
        sudo dnf install -y lcov
    elif command_exists brew; then
        echo "Detected macOS. Installing lcov using Homebrew..."
        brew install lcov
    else
        echo "Unsupported operating system. Please install lcov manually."
        exit 1
    fi
}

# Check if lcov is installed
if ! command_exists lcov; then
    install_lcov
else
    echo "lcov is already installed."
fi

# Check if gcov and gcc versions match
echo "Checking gcov and gcc versions..."
gcov_version=$(gcov --version | head -n 1)
gcc_version=$(gcc --version | head -n 1)

echo "gcov version: $gcov_version"
echo "gcc version: $gcc_version"

if [[ "$gcov_version" != *"${gcc_version#* }"* ]]; then
    echo "Warning: gcov and gcc versions do not match. Ensure they are consistent."
fi

# Build the project
echo "Building the project..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run tests
echo "Running tests..."
ctest

# Generate code coverage report
echo "Generating code coverage report..."
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.filtered.info
genhtml coverage.filtered.info --output-directory coverage_report

# Open the coverage report
echo "Opening the coverage report..."
xdg-open coverage_report/index.html || open coverage_report/index.html || echo "Please open coverage_report/index.html manually."

echo "Done!"