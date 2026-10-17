#!/usr/bin/env bash
# Dispatch tests for proprietary/wrapper.sh — no CUPS, no printing.
# The ARM QEMU success path needs the real i386 runtime; scripts/verify.sh
# covers it on the test box. Here: dispatch, failure modes, and — where the
# runtime exists — that the ARM path reaches qemu.
set -euo pipefail

root="$(cd "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
mkdir -p "$tmp/bin" "$tmp/fake"

install -m 0755 "$root/proprietary/wrapper.sh" "$tmp/bin/rastertokpsl"
for b in rastertokpsl_x86 rastertokpsl_amd64; do
  printf '#!/bin/sh\necho "%s: $*"\n' "$b" > "$tmp/bin/$b"
  chmod +x "$tmp/bin/$b"
done

fake_arch() {
  printf '#!/bin/sh\necho %s\n' "$1" > "$tmp/fake/uname"
  chmod +x "$tmp/fake/uname"
}

run() { PATH="$tmp/fake:$PATH" "$tmp/bin/rastertokpsl" doc.pdf u "My Job!" 1 o 2>&1; }

fail() {
  echo "FAIL: $1" >&2
  exit 1
}

fake_arch x86_64
out=$(run) || fail "x86_64 exited nonzero: $out"
[[ $out == *rastertokpsl_amd64* ]] || fail "x86_64 did not pick amd64: $out"
[[ $out == *"MyJob"* ]] || fail "title not sanitized: $out"

fake_arch i686
out=$(run) || fail "i686 exited nonzero: $out"
[[ $out == *rastertokpsl_x86* ]] || fail "i686 did not pick x86: $out"

# The wrapper's ARM preflight depends on the host: where the QEMU runtime
# is installed (the ARM test box) it passes and the wrapper hands the
# non-i386 stub to qemu; elsewhere it must report the missing runtime.
fake_arch armv7l
qemu_ready=0
for q in qemu-i386 qemu-i386-static; do
  if command -v "$q" >/dev/null 2>&1; then
    qemu_ready=1
    break
  fi
done
if [ "$qemu_ready" = 1 ] &&
  [ -e /lib/ld-linux.so.2 ] &&
  [ -e /usr/lib/i386-linux-gnu/libcups.so.2 ] &&
  [ -e /usr/lib/i386-linux-gnu/libcupsimage.so.2 ]; then
  if out=$(run); then
    fail "armv7l stub unexpectedly ran: $out"
  fi
  [[ $out == *rastertokpsl_x86* ]] || fail "armv7l did not reach qemu: $out"
else
  if out=$(run); then
    fail "armv7l without a QEMU runtime must fail: $out"
  fi
  [[ $out == *QEMU* ]] || fail "armv7l wrong error: $out"
fi

fake_arch riscv64
if out=$(run); then
  fail "unsupported arch must fail: $out"
fi
[[ $out == *unsupported* ]] || fail "unsupported arch wrong error: $out"

fake_arch x86_64
chmod -x "$tmp/bin/rastertokpsl_amd64"
if out=$(run); then
  fail "missing native binary must fail: $out"
fi
[[ $out == *missing* ]] || fail "missing binary wrong error: $out"

echo "OK: wrapper dispatch tests passed"
