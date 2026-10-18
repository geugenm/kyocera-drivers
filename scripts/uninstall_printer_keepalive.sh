#!/usr/bin/env bash
# uninstall_printer_keepalive.sh — remove the Kyocera USB keepalive daemon.
# Leaves the CUPS queue, shared printing and the usblp module untouched.

set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "needs root: sudo $0" >&2
    exit 1
fi

systemctl disable --now kyocera-keepalive.timer 2>/dev/null || true
systemctl stop kyocera-keepalive.service 2>/dev/null || true
systemctl reset-failed kyocera-keepalive.service 2>/dev/null || true
rm -f /usr/local/sbin/kyocera-keepalive \
      /etc/systemd/system/kyocera-keepalive.service \
      /etc/systemd/system/kyocera-keepalive.timer \
      /etc/default/kyocera-keepalive \
      /etc/modules-load.d/kyocera-keepalive.conf \
      /etc/udev/rules.d/69-kyocera-keepalive.rules
systemctl daemon-reload
udevadm control --reload 2>/dev/null || true

echo "removed: keepalive service, timer, config, udev rule, modules-load entry"
echo "the CUPS queue and shared printing are untouched"
