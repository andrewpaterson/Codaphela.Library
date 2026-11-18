#!/bin/bash
# gcccheck.sh <directory of exported cases>: each case with g++ -std=c++14, as hostcheck.py does with the Microsoft compiler.
cd "$1" || exit 1
out=/tmp/exccases
mkdir -p $out
checked=0
wrong=0
while read name value; do
    checked=$((checked + 1))
    if g++ -std=c++14 -w -o $out/a.out "$name.cpp" 2> $out/err.txt; then
        if [ "$value" = "-1" ]; then
            echo "$name must be refused, and compiles"
            wrong=$((wrong + 1))
            continue
        fi
        timeout 20 $out/a.out > /dev/null 2>&1
        returned=$?
        if [ "$returned" != "$value" ]; then
            echo "$name returns $returned not $value"
            wrong=$((wrong + 1))
        fi
    elif [ "$value" != "-1" ]; then
        echo "$name does not compile: $(grep error $out/err.txt | head -2 | cut -c1-200)"
        wrong=$((wrong + 1))
    fi
done < Cases.txt
echo "$checked cases checked, $wrong wrong"
g++ --version | head -1
