#!/usr/bin/env bash
set -euo pipefail

input_file="$1"
user="$2"
title="$3"
copies="$4"
options="$5"

# Force the C locale: tr then treats the title byte-wise, so multibyte
# UTF-8 characters (e.g. Cyrillic) can never leak into Kyocera's binary,
# which aborts on non-ASCII input. See "Known Upstream Problems" in readme.
export LC_ALL=C

job_name=$(echo "$title" | tr -cd '[:alnum:]' | tail -c 20)

# ------------------------------------------------------------------
# Filter resolution
# ------------------------------------------------------------------
# Kyocera's proprietary binary is preferred wherever it can run:
#   x86/x86_64 — natively;
#   ARM, etc.  — through qemu-i386, since Kyocera never shipped an ARM
#                build (see the readme's ARM section).
# The open-source rastertokpsl-re filter is the fallback when no
# proprietary binary can run at all.
arch=$(uname -m)
case "$arch" in
x86_64 | amd64)
  native="rastertokpsl_amd64"
  ;;
i386 | i486 | i586 | i686)
  native="rastertokpsl_x86"
  ;;
armv6l | armv7l | armv8l | aarch64 | arm64)
  native=""
  ;;
*)
  echo "[wrapper.sh] error: unsupported architecture '$arch'" >&2
  exit 1
  ;;
esac

# Filters are installed next to this wrapper. Resolve them relative to
# the script's own location so the install works under any prefix:
# /usr, /usr/local, or a --prefix override at install time.
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

run() {
  exec "$@" "$input_file" "$user" "$job_name" "$copies" "$options"
}

# 1. Architecture-native proprietary binary.
if [ -n "$native" ] && [ -x "$script_dir/$native" ]; then
  run "$script_dir/$native"
fi

# 2. Kyocera's 32-bit x86 binary through qemu-i386 (ARM and other hosts).
if [ -z "$native" ] && [ -x "$script_dir/rastertokpsl_x86" ]; then
  qemu=""
  for q in qemu-i386 qemu-i386-static; do
    if command -v "$q" >/dev/null 2>&1; then
      qemu="$q"
      break
    fi
  done
  if [ -n "$qemu" ] &&
    [ -e /lib/ld-linux.so.2 ] &&
    [ -e /usr/lib/i386-linux-gnu/libcups.so.2 ] &&
    [ -e /usr/lib/i386-linux-gnu/libcupsimage.so.2 ]; then
    run "$qemu" "$script_dir/rastertokpsl_x86"
  fi
  echo "[wrapper.sh] note: QEMU i386 runtime unavailable; falling back" >&2
fi

# 3. Open-source rastertokpsl-re — native alternative used when the
#    proprietary binary cannot run (ARM without QEMU) or was replaced.
if [ -x "$script_dir/rastertokpsl-re" ]; then
  run "$script_dir/rastertokpsl-re"
fi
if command -v rastertokpsl-re >/dev/null 2>&1; then
  run rastertokpsl-re
fi

echo "[wrapper.sh] error: no usable raster filter for '$arch'" >&2
echo "  To run Kyocera's x86 filter under emulation (ARM):" >&2
echo "    sudo dpkg --add-architecture i386 && sudo apt update" >&2
echo "    sudo apt install qemu-user-static libc6:i386 libcups2:i386 libcupsimage2:i386" >&2
echo "  Or install the open-source replacement as 'rastertokpsl-re':" >&2
echo "    https://github.com/Fe-Ti/rastertokpsl-re" >&2
exit 1
