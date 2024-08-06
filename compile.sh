#! /usr/bin/env bash

set -euo pipefail

HERE=$(readlink -f $(dirname -- "$BASH_SOURCE"))

# app

APP_NAME='chess-4way'

WASM='y'

# compiler

if [ "$WASM" == 'n' ]; then
	COMPILER='g++'
else
	COMPILER='em++'
fi

# compiler flags

FLAGS_STANDARD='-std=c++23'

FLAGS_COMPILETIME_SAFETY='-Werror -Wextra -Wall -pedantic -Wfatal-errors -Wshadow -Wconversion'
# those do not affect the runtime performance

FLAGS_REDUCE_COMPILITIME_SAFETY='-Wno-sign-conversion -Wno-implicit-int-conversion -Wno-limited-postlink-optimizations'

FLAGS_RUNTIME_SAFETY=''
# those do affect the runtime performance
# example: -fsanitize=undefined
# -fsanitize=undefined will detect undefined behaviour at runtime (example: signed overflow)

FLAGS_OPTIMISATION='-Ofast'
if [ "$WASM" != 'y' ]; then
	FLAGS_OPTIMISATION="$FLAGS_OPTIMISATION -march=native"
fi

FLAGS_MISC='-g -static'
# note: can't have both -static and -fsanitize=undefined

FLAGS_LIB=''
# example: -lgmp

FLAGS="$FLAGS_STANDARD $FLAGS_COMPILETIME_SAFETY $FLAGS_REDUCE_COMPILITIME_SAFETY $FLAGS_RUNTIME_SAFETY $FLAGS_OPTIMISATION $FLAGS_MISC $FLAGS_LIB"

# set define containing current git commit

DEFINE_COMMIT_ID=$(git -C "$HERE" rev-parse HEAD)

# output file

if [ "$WASM" == 'n' ]; then
	OUTPUT="$HERE/$APP_NAME"
else
	rm -rf "$HERE/wasm-output"
	mkdir "$HERE/wasm-output"
	OUTPUT="$HERE/wasm-output/$APP_NAME.html"
fi

# compile

$COMPILER $FLAGS -D COMMIT_ID="\"$DEFINE_COMMIT_ID\"" -o "$OUTPUT" "$HERE/$APP_NAME.cpp"

echo "Compiled binary:"
echo "$OUTPUT"
