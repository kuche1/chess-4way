#! /usr/bin/env bash

set -euo pipefail

HERE=$(readlink -f $(dirname -- "$BASH_SOURCE"))

# app

APP_NAME='chess-4way'

# compiler flags

FLAGS_STANDARD='-std=c++23'

FLAGS_COMPILETIME_SAFETY='-Werror -Wextra -Wall -pedantic -Wfatal-errors -Wshadow -Wconversion'
# those do not affect the runtime performance

FLAGS_RUNTIME_SAFETY=''
# those do affect the runtime performance
# example: -fsanitize=undefined
# -fsanitize=undefined will detect undefined behaviour at runtime (example: signed overflow)

FLAGS_OPTIMISATION='-Ofast -march=native'
# example: -Ofast -march=native

FLAGS_MISC='-g'
# example: -static
# note: can't have both -static and -fsanitize=undefined

FLAGS_LIB=''
# example: -lgmp

FLAGS="$FLAGS_STANDARD $FLAGS_COMPILETIME_SAFETY $FLAGS_RUNTIME_SAFETY $FLAGS_OPTIMISATION $FLAGS_MISC $FLAGS_LIB"

# set define containing current git commit

DEFINE_COMMIT_ID=$(git -C "$HERE" rev-parse HEAD)

# compile

output="$HERE/$APP_NAME"

g++ $FLAGS -DCOMMIT_ID="\"$DEFINE_COMMIT_ID\"" -o "$output" "$HERE/$APP_NAME.cpp"

echo "Compiled binary:"
echo "$output"
