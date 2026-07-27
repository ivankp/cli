#!/usr/bin/env bash

set -e

cd "${0%/*}" # cd to the directory containing this script
mkdir -p build

deps=(testing.h ../include/cli.h ../src/cli.c)

compilers=()
stds=()
suites=()

for arg in "$@"; do
    if [[ "$arg" =~ ^c(\+\+)?[0-9]{2}$ ]]; then
        stds+=("$arg")
    elif [[ "$arg" =~ \.c(c|pp|xx|\+\+)$ ]] && [ -f "$arg" ]; then
        suites+=("$arg")
    elif command -v "$arg" >/dev/null 2>&1; then
        compilers+=("$arg")
    else
        echo "Unexpected argument \"$arg\""
        exit 1
    fi
done

if [ ${#compilers[@]} -eq 0 ]; then
    for comp in gcc clang cl; do
        if command -v "$comp" >/dev/null 2>&1; then
            compilers+=("$comp")
        fi
    done
fi
if [ ${#compilers[@]} -eq 0 ]; then
    echo "Unable to find a compiler"
    exit 1
fi

[ ${#stds[@]} -eq 0 ] && stds=(c99 c11 c23)

[ ${#suites[@]} -eq 0 ] && suites=(tests-*.c)
if [ ${#suites[@]} -eq 0 ]; then
    echo "Unable to find a test suite"
    exit 1
fi

exes=()
pids=()

srcs=()
for dep in "${deps[@]}"; do
    [[ "$dep" == *.c ]] && srcs+=("$dep")
done

for suite in "${suites[@]}"; do
    newest="${0##*/}" # this script
    for src in "$suite" "${deps[@]}"; do
        if [ "$src" -nt "$newest" ]; then
            newest="$src"
        fi
    done

    for comp in "${compilers[@]}"; do
        if [ "$comp" = 'gcc' ]; then
            nerr='-fmax-errors=3'
        elif [ "$comp" = 'clang' ]; then
            nerr='-ferror-limit=3'
        else
            unset nerr
        fi
        for std in "${stds[@]}"; do
            exe="build/${suite%%.c}-$std-$comp"
            exes+=("$exe")
            if [ ! -f "$exe" ] || [ "$newest" -nt "$exe" ]; then
                if [ "$comp" == 'cl' ]; then
                    cmd=("$comp" \
                        /EHsc \
                        /std:"$std" \
                        /W4 /WX /wd4702 \
                        /O2 \
                        /I. /I../include \
                        /Fe"$exe" "${srcs[@]}" "$suite")
                else
                    cmd=("$comp" \
                        -std="$std" \
                        -Wall -Wextra -Werror -pedantic \
                        $nerr \
                        -I. -I../include \
                        -DCLI_UNIT_TEST \
                        "${srcs[@]}" "$suite" -o "$exe")
                fi
                echo -e '\033[34m'"${cmd[@]}"'\033[0m'
                if [ "$comp" == 'cl' ]; then
                    "${cmd[@]}"
                else
                    "${cmd[@]}" &
                fi
                pids+=($!)
            fi
        done
    done
done

for pid in ${pids[@]}; do
    wait $pid || exit 1
done

for exe in "${exes[@]}"; do
    echo "$exe"
    ./"$exe"
done
