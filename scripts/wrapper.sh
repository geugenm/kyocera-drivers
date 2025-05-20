#!/usr/bin/env bash

set -euo pipefail

job_name=$(printf '%s' "$3" | grep -o '[[:alnum:]]' | tr -d '\n' | tail -c 20)

exec /usr/lib/cups/filter/rastertokpsl-bin "$1" "$2" "$job_name" "$4" "$5"

