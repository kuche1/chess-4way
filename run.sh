#! /usr/bin/env bash

set -euo pipefail

HERE=$(readlink -f $(dirname -- "$BASH_SOURCE"))

executable=$("$HERE/compile.sh" | tail -n 1)

"$executable"
