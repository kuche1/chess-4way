#! /usr/bin/env bash

set -euo pipefail

HERE=$(readlink -f $(dirname -- "$BASH_SOURCE"))

# app

APP_NAME='chess-4way'

# compiler flags

FLAGS_STANDARD='-std=c++23'

FLAGS_STRICT='-Werror -Wextra -Wall -pedantic -Wfatal-errors -Wshadow -Wconversion -fsanitize=undefined'
# -fsanitize=undefined will detect undefined behaviour at runtime (example: signed overflow)

FLAGS_OPTIMISATION=''
# example: -Ofast -march=native

FLAGS_MISC=''
# example: -static
# note: can't have both -static and -fsanitize=undefined

FLAGS_LIB=''
# example: -lgmp

FLAGS="$FLAGS_STANDARD $FLAGS_STRICT $FLAGS_OPTIMISATION $FLAGS_MISC $FLAGS_LIB"

# set define containing current git commit

DEFINE_COMMIT_ID=$(git -C "$HERE" rev-parse HEAD)

# compile

output="$HERE/$APP_NAME"

g++ $FLAGS -DCOMMIT_ID="\"$DEFINE_COMMIT_ID\"" -o "$output" "$HERE/$APP_NAME.cpp"

echo "Compiled binary:"
echo "$output"
