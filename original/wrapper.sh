#!/usr/bin/env bash
set -euo pipefail

input_file="$1"
user="$2"
title="$3"
copies="$4"
options="$5"

job_name=$(echo "$title" | grep -o '[[:alnum:]]' | tr -d '\n' | tail -c 20)

arch=$(uname -m)
case "$arch" in
x86_64 | amd64)
  bin="rastertokpsl-x64"
  ;;
i386 | i486 | i586 | i686)
  bin="rastertokpsl-x32"
  ;;
*)
  echo "[wrapper.sh] error: unsupported architecture '$arch'" >&2
  exit 1
  ;;
esac

exec "/usr/lib/cups/filter/$bin" "$input_file" "$user" "$job_name" "$copies" "$options"
