#!/usr/bin/env bash
# One-shot setup for a fresh machine, from this checkout:
# system packages (incl. the QEMU i386 runtime on ARM), build, install,
# CUPS restart, then a non-printing verification.
#
# Usage: sudo bash scripts/setup.sh   (root directly also works)
set -euo pipefail

cd "$(dirname -- "${BASH_SOURCE[0]}")/.."
if [ ! -f CMakeLists.txt ]; then
  echo "error: run this from a kyocera-drivers checkout" >&2
  exit 1
fi

SUDO=""
[ "$(id -u)" = 0 ] || SUDO="sudo"

arch=$(uname -m)
case "$arch" in
x86_64 | amd64 | i386 | i486 | i586 | i686)
  echo "==> x86 host ($arch): installing build tools and CUPS stack"
  $SUDO apt-get install -y cmake ninja-build cups cups-filters ghostscript
  ;;
armv6l | armv7l | armv8l | aarch64 | arm64)
  echo "==> ARM host ($arch): installing the CUPS stack and the QEMU i386 runtime for Kyocera's x86 filter"
  $SUDO dpkg --add-architecture i386
  $SUDO apt-get update
  $SUDO apt-get install -y cmake ninja-build cups cups-filters ghostscript qemu-user-static \
    libc6:i386 libcups2:i386 libcupsimage2:i386
  $SUDO ln -sf /lib/i386-linux-gnu/ld-linux.so.2 /lib/ld-linux.so.2
  ;;
*)
  echo "error: unsupported host architecture '$arch'" >&2
  exit 1
  ;;
esac

echo "==> building and installing"
cmake --preset default
$SUDO cmake --install build/default --prefix=/usr

echo "==> restarting CUPS"
$SUDO systemctl restart cups 2>/dev/null || echo "warning: could not restart CUPS — do it manually"

echo "==> verifying (no printing)"
bash scripts/verify.sh
