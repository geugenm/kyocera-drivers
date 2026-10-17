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

# Filter resolution order:
#   1. rastertokpsl-re — the open-source replacement. It is the only
#      option on architectures Kyocera never shipped (ARM) and a drop-in
#      fix for the proprietary binary's non-ASCII crash on x86.
#   2. the architecture-native proprietary binary.
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

# Prefer filters installed next to this wrapper — the install works under
# any prefix: /usr, /usr/local, or a --prefix override at install time —
# but accept rastertokpsl-re from PATH too (it may be packaged separately).
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

for bin in rastertokpsl-re $native; do
  [ -n "$bin" ] || continue
  if [ -x "$script_dir/$bin" ]; then
    exec "$script_dir/$bin" "$input_file" "$user" "$job_name" "$copies" "$options"
  fi
  if command -v "$bin" >/dev/null 2>&1; then
    exec "$bin" "$input_file" "$user" "$job_name" "$copies" "$options"
  fi
done

echo "[wrapper.sh] error: no raster filter available for '$arch'" >&2
echo "  Install the open-source replacement as 'rastertokpsl-re'" >&2
echo "  next to this wrapper (or anywhere in PATH):" >&2
echo "    https://github.com/Fe-Ti/rastertokpsl-re" >&2
exit 1
