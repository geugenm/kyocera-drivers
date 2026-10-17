#!/usr/bin/env bash
# CUPS dispatcher for Kyocera's proprietary rastertokpsl filter.
#
# Runs the real filter for this machine — natively on x86, through
# qemu-i386 on ARM (Kyocera never shipped an ARM build) — and sanitizes
# the job title, which Kyocera's binary crashes on when it contains
# non-ASCII bytes (leaves the printer stuck until a power cycle).
set -euo pipefail

input_file="$1"
user="$2"
title="$3"
copies="$4"
options="$5"

export LC_ALL=C
job_name=$(echo "$title" | tr -cd '[:alnum:]' | tail -c 20)

# The real filters are installed next to this wrapper, so any install
# prefix works (/usr, /usr/local, --prefix override).
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

arch=$(uname -m)
case "$arch" in
x86_64 | amd64)
  bin="$script_dir/rastertokpsl_amd64"
  runner=""
  ;;
i386 | i486 | i586 | i686)
  bin="$script_dir/rastertokpsl_x86"
  runner=""
  ;;
armv6l | armv7l | armv8l | aarch64 | arm64)
  # No ARM build exists: run the 32-bit binary under QEMU. The runtime
  # is installed by scripts/setup.sh (qemu-user-static + i386 cups libs).
  bin="$script_dir/rastertokpsl_x86"
  runner=""
  for q in qemu-i386 qemu-i386-static; do
    if command -v "$q" >/dev/null 2>&1; then
      runner="$q"
      break
    fi
  done
  if [ -z "$runner" ] || [ ! -e /lib/ld-linux.so.2 ] ||
    [ ! -e /usr/lib/i386-linux-gnu/libcups.so.2 ] ||
    [ ! -e /usr/lib/i386-linux-gnu/libcupsimage.so.2 ]; then
    echo "[wrapper.sh] error: QEMU i386 runtime incomplete — see readme (scripts/setup.sh)" >&2
    exit 1
  fi
  ;;
*)
  echo "[wrapper.sh] error: unsupported architecture '$arch'" >&2
  exit 1
  ;;
esac

if [ ! -x "$bin" ]; then
  echo "[wrapper.sh] error: $(basename "$bin") missing next to the wrapper — reinstall" >&2
  exit 1
fi

argv=("$bin")
if [ -n "$runner" ]; then
  argv=("$runner" "$bin")
fi
exec "${argv[@]}" "$input_file" "$user" "$job_name" "$copies" "$options"
