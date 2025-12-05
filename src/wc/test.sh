#!/bin/bash

if [ ! -z $1 ] && [ $1 = "clean" ]; then
    if [ ! -d "./tests" ]; then
        echo "No test files to clean."
        exit 1
    fi
    echo "Cleaning test files..."
    rm -rf tests
    if [ $? -ne 0 ]; then
        echo "Failed to clean test files."
        exit 1
    fi
    printf "\rCleaned test files.\n"
    exit 0
fi

if [ ! -d "./tests" ]; then
    echo "Creating tests directory..."
    mkdir -p tests
    if [ $? -ne 0 ]; then
        echo "Failed to create tests directory."
        exit 1
    fi
fi

if [ ! -f "tests/random_string" ]; then
    echo "Compiling random_string generator..."
    g++ -O3 -march=native -pipe -flto -DNDEBUG -pedantic -o tests/random_string random_string.cpp && strip tests/random_string
    if [ $? -ne 0 ]; then
        echo "Failed to compile random_string generator."
        exit 1
    fi
fi

echo "Generating test file..."
tests/random_string
if [ $? -ne 0 ]; then
    echo "Failed to generate test file."
    exit 1
fi

printf "Compiling wc..."
gcc -O3 -march=native -pipe -flto -DNDEBUG -pedantic -o wc wc.c
if [ $? -ne 0 ]; then
    echo "Failed to compile wc."
    exit 1
fi
printf "\rFinished Compiling!\n"
strip wc && echo -e "Executable stripped!, running tests...\n"

printf "tests/ascii.txt\0" > tests/file_list.txt
printf "tests/utf8.txt\0" >> tests/file_list.txt
printf "tests/utf8-ascii-mix.txt\0" >> tests/file_list.txt

echo "Testing wc with --files0-from (stdin) option..."
printf "tests/ascii.txt\0" | ./wc -cmlLw --total=always --files0-from=-
if [ $? -ne 0 ]; then
    echo "wc test with --files0-from (stdin) failed."
    exit 1
else
    times
fi
echo "Testing wc with --files0-from (file) option..."
time ./wc -cmlLw --total=always --files0-from=tests/file_list.txt
if [ $? -ne 0 ]; then
    echo "wc test with --files0-from (file) failed."
    exit 1
fi

echo -e "\nRunning benchmark on GNU's wc..."
time wc -cmlLw --total=always --files0-from=tests/file_list.txt
if [ $? -ne 0 ]; then
    echo "GNU wc benchmark failed."
    exit 1
else
    times
fi
echo "Benchmark completed."