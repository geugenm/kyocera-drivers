#!/usr/bin/env bash
# install-printer-keepalive.sh — install the Kyocera "printer not going
# dead on USB" daemon on this host: usb keepalive (oneshot service + timer).
#
# Installs:
#   /usr/local/sbin/kyocera-keepalive              the probe itself
#   /etc/systemd/system/kyocera-keepalive.service  oneshot unit
#   /etc/systemd/system/kyocera-keepalive.timer    every KEEPALIVE_INTERVAL
#   /etc/default/kyocera-keepalive                 queue/recover settings
#   /etc/modules-load.d/kyocera-keepalive.conf     loads usblp at boot
#   /etc/udev/rules.d/69-kyocera-keepalive.rules   never autosuspend printer
#
# Usage: sudo ./install-printer-keepalive.sh
# Optional interval: KEEPALIVE_INTERVAL=5min sudo -E ./install-printer-keepalive.sh

set -euo pipefail

interval="${KEEPALIVE_INTERVAL:-2min}"
if ! [[ "${interval}" =~ ^[0-9]+(min|s|h)$ ]]; then
    echo "invalid KEEPALIVE_INTERVAL '${interval}' (formats: 300s, 10min, 1h)" >&2
    exit 1
fi

BIN=/usr/local/sbin/kyocera-keepalive
DEFAULT_FILE=/etc/default/kyocera-keepalive
SVC=/etc/systemd/system/kyocera-keepalive.service
TIMER=/etc/systemd/system/kyocera-keepalive.timer
MLOAD=/etc/modules-load.d/kyocera-keepalive.conf
UDEV=/etc/udev/rules.d/69-kyocera-keepalive.rules

if [ "$(id -u)" -ne 0 ]; then
    echo "needs root: sudo $0" >&2
    exit 1
fi

# Locate the printer's usblp node and its USB ids. The vendor/product pair
# 0482:0495 is the Kyocera FS-1020MFP; fall back to the first node only with
# a warning so one-printer boxes still work after CUPS re-claims the device.
usb_device_dir() {
    local syspath dir i
    syspath="$(udevadm info -q path --name="$1" 2>/dev/null || true)"
    case "${syspath}" in
        /sys*) dir="${syspath}" ;;
        *) dir="/sys${syspath}" ;;
    esac
    for i in 1 2 3 4 5; do
        if [ -f "${dir}/idVendor" ]; then
            echo "${dir}"
            return 0
        fi
        dir="$(dirname "${dir}")"
    done
    return 1
}

lp_device=""
usb_vendor=""
usb_product=""
fallback_lp=""
fallback_vendor=""
fallback_product=""
for lp in /dev/usb/lp*; do
    [ -e "${lp}" ] || continue
    vendor=""
    product=""
    if usb_dir="$(usb_device_dir "${lp}")"; then
        vendor="$(cat "${usb_dir}/idVendor" 2>/dev/null || true)"
        product="$(cat "${usb_dir}/idProduct" 2>/dev/null || true)"
    fi
    if [ -z "${fallback_lp}" ]; then
        fallback_lp="${lp}"
        fallback_vendor="${vendor}"
        fallback_product="${product}"
    fi
    if [ "${vendor}" = "0482" ] && [ "${product}" = "0495" ]; then
        lp_device="${lp}"
        usb_vendor="${vendor}"
        usb_product="${product}"
        break
    fi
done
if [ -z "${lp_device}" ]; then
    if [ -z "${fallback_lp}" ]; then
        echo "error: no /dev/usb/lp* node; is the printer connected and usblp loaded?" >&2
        exit 1
    fi
    lp_device="${fallback_lp}"
    usb_vendor="${fallback_vendor}"
    usb_product="${fallback_product}"
    echo "note: no Kyocera 0482:0495 pair found; using ${lp_device} (vendor ${usb_vendor:-?}:${usb_product:-?})" >&2
fi

queue="$(lpstat -v 2>/dev/null | sed -n 's|^\([^:]*\): usb://Kyocera/.*|\1|p' | awk '{print $NF}' | head -n1 || true)"
if [ -z "${queue}" ]; then
    queue="FS-1020MFP"
    echo "note: no CUPS usb://Kyocera queue found; assuming '${queue}'" >&2
fi

cat >"${BIN}" <<'EOF_KEEPALIVE'
#!/usr/bin/env bash
# kyocera-keepalive — keep the Kyocera FS-1020MFP's usb link alive.
#
# How it talks to the printer (nothing here can ever form a page):
#   1. every cycle asks the printer who it is through CUPS' own usb backend
#      in enumeration mode — the exact open-time interaction of every real
#      print job: the kernel printer driver churns out and back and the
#      printer answers with its IEEE-1284 device-id string. A real answer,
#      zero printable bytes.
#   2. if the printer stops answering: rescan the bus, re-enumerate the port
#      host-side (the automated equivalent of a usb cable re-plug), ask again.
#   3. if an earlier usb blackout left the CUPS queue parked (disabled,
#      stopped), re-enable it so user jobs never stay stranded.
set -euo pipefail

backend="${KYOCERA_KEEPALIVE_BACKEND:-/usr/lib/cups/backend/usb}"
queue="${KYOCERA_KEEPALIVE_QUEUE:-FS-1020MFP}"
auto_recover="${KYOCERA_KEEPALIVE_AUTO_RECOVER:-1}"

# A live CUPS job already exercises the printer; never interleave with it.
pending="$(lpstat -o "${queue}" 2>/dev/null || true)"
if [ -n "${pending}" ]; then
    echo "${queue}: jobs in queue, skipping this cycle"
    exit 0
fi

node="$(ls /dev/usb/lp* 2>/dev/null | head -n1 || true)"
if [ -z "${node}" ] || [ ! -e "${node}" ]; then
    echo "no /dev/usb/lp* node; printer off the bus?" >&2
    exit 1
fi

# The printer's own answer: its IEEE-1284 device id. CUPS' usb backend in
# enumeration mode opens/claims the interface exactly like a real job does.
printer_answers() {
    [ -x "${backend}" ] || { echo "${backend}: cups usb backend not found" >&2; return 1; }
    timeout 25 "${backend}" 2>/dev/null | grep -i 'usb://.*kyocera' | head -n1
}

# udevadm reports syspaths without the /sys prefix; walk up to the usb
# device dir that carries the attributes.
usb_device_dir() {
    local syspath dir i
    syspath="$(udevadm info -q path --name="$1" 2>/dev/null || true)"
    case "${syspath}" in
        /sys*) dir="${syspath}" ;;
        *) dir="/sys${syspath}" ;;
    esac
    for i in 1 2 3 4 5; do
        if [ -f "${dir}/idVendor" ]; then
            echo "${dir}"
            return 0
        fi
        dir="$(dirname "${dir}")"
    done
    return 1
}

# Host-side re-enumeration (authorized 0 -> 1): the automated equivalent
# of unplugging and replugging the usb cable.
reenumerate() {
    local authorized
    authorized="$(usb_device_dir "$1" 2>/dev/null || true)/authorized"
    if [ ! -e "${authorized}" ]; then
        echo "cannot re-enumerate: device dir for $1 not found" >&2
        return 1
    fi
    echo "host-side re-enumeration of the printer port" >&2
    echo 0 >"${authorized}" || true
    sleep 2
    echo 1 >"${authorized}" || true
    sleep 3
    return 0
}

# Never let an earlier blackout strand the user: CUPS can park the queue
# (disabled/stopped) after a failed job. lpstat text is localized, so
# match states, not words.
ensure_queue_enabled() {
    if lpstat -p "${queue}" 2>/dev/null | grep -qiE 'disabled|stopped|неактив|останов'; then
        echo "re-enabling cups queue ${queue}"
        cupsenable "${queue}" 2>/dev/null || true
        cupsaccept "${queue}" 2>/dev/null || true
    fi
    return 0
}

if answer="$(printer_answers)"; then
    echo "keepalive ok: printer answered: ${answer}"
    ensure_queue_enabled
    exit 0
fi

# Failure path: rescan, re-enumerate and ask again.
echo "printer did not answer; rescanning usb bus" >&2
lpinfo -v 2>/dev/null | grep -i kyocera || echo "scan: Kyocera not on the usb bus" >&2
if [ "${auto_recover}" = "1" ]; then
    node="$(ls /dev/usb/lp* 2>/dev/null | head -n1 || true)"
    if [ -n "${node}" ] && [ -e "${node}" ]; then
        reenumerate "${node}" || true
        sleep 1
        if answer="$(printer_answers)"; then
            echo "keepalive recovered after re-enumeration: ${answer}"
            ensure_queue_enabled
            exit 0
        fi
    fi
fi
ensure_queue_enabled
echo "printer is not answering; will retry next cycle" >&2
exit 1
EOF_KEEPALIVE
chmod 755 "${BIN}"
bash -n "${BIN}"

cat >"${DEFAULT_FILE}" <<EOF_DEFAULTS
# Settings read by kyocera-keepalive.service before each run.
KYOCERA_KEEPALIVE_QUEUE=${queue}
# 1 = when the printer stops answering: re-scan the bus, re-enumerate the
#     usb port and retry; 0 = wait for the next cycle instead
KYOCERA_KEEPALIVE_AUTO_RECOVER=1
# Only if the backend lives somewhere else:
#KYOCERA_KEEPALIVE_BACKEND=/usr/lib/cups/backend/usb
EOF_DEFAULTS

cat >"${SVC}" <<'EOF_SERVICE'
[Unit]
Description=Keep Kyocera USB printer out of un-wakeable deep sleep (status-only probes)
After=printer.target

[Service]
Type=oneshot
EnvironmentFile=/etc/default/kyocera-keepalive
ExecStartPre=-/usr/sbin/modprobe --quiet usblp
ExecStart=/usr/local/sbin/kyocera-keepalive
EOF_SERVICE

cat >"${TIMER}" <<EOF_TIMER
[Unit]
Description=Run the Kyocera USB printer keepalive every ${interval}

[Timer]
OnBootSec=2min
OnUnitActiveSec=${interval}
AccuracySec=15s
Unit=kyocera-keepalive.service

[Install]
WantedBy=timers.target
EOF_TIMER

cat >"${MLOAD}" <<'EOF_MLOAD'
usblp
EOF_MLOAD

if [ -n "${usb_vendor}" ] && [ -n "${usb_product}" ]; then
    cat >"${UDEV}" <<EOF_UDEV
# Kyocera FS-1020MFP (usb ${usb_vendor}:${usb_product}): never autosuspend
# the device; the keepalive daemon handles idle wakeups.
ACTION=="add|change", ATTRS{idVendor}=="${usb_vendor}", ATTRS{idProduct}=="${usb_product}", TEST=="power/control", ATTR{power/control}="on", TEST=="power/autosuspend_delay_ms", ATTR{power/autosuspend_delay_ms}="-1"
EOF_UDEV
fi

# Disable kernel autosuspend for the device right away, not only on next
# hotplug, and make sure the usblp module is up for the first probe.
sysdir="$(usb_device_dir "${lp_device}" || true)"
if [ -d "${sysdir}/power" ]; then
    [ -f "${sysdir}/power/control" ] && echo on >"${sysdir}/power/control" || true
    [ -f "${sysdir}/power/autosuspend_delay_ms" ] && echo -1 >"${sysdir}/power/autosuspend_delay_ms" || true
fi
if ! lsmod | grep -q '^usblp ' 2>/dev/null; then
    modprobe usblp 2>/dev/null || echo "note: modprobe usblp failed (kept for boot via modules-load.d)" >&2
fi
if [ -e "${UDEV}" ]; then
    udevadm control --reload
fi

chmod 644 "${DEFAULT_FILE}" "${SVC}" "${TIMER}" "${MLOAD}"
[ ! -e "${UDEV}" ] || chmod 644 "${UDEV}"

systemctl daemon-reload

# Failed jobs should retry through a usb blackout, not strangle the queue.
lpadmin -p "${queue}" -o printer-error-policy=retry-job 2>/dev/null || \
    echo "note: could not set retry-job error policy on ${queue}" >&2

systemctl enable --now kyocera-keepalive.timer
systemctl start kyocera-keepalive.service || \
    echo "warning: first keepalive run failed; see: journalctl -u kyocera-keepalive -n 20" >&2

echo "printer keepalive installed:"
echo "  printer: ${lp_device} (usb ${usb_vendor:-?}:${usb_product:-?})"
echo "  queue:   ${queue} (error policy: retry-job)"
echo "  probe:   every ${interval} — asks the printer for its 1284 device id"
echo "           through the cups usb backend; nothing printable is ever sent"
echo "  recover: re-enumerates the usb port and re-enables the queue on failure"
echo "  remove:  sudo $(dirname "$0")/uninstall-printer-keepalive.sh"
echo "checks:"
echo "  systemctl list-timers kyocera-keepalive.timer"
echo "  journalctl -u kyocera-keepalive -n 20"
echo "config: ${DEFAULT_FILE} (queue/recover; applied on next run)"
echo "interval: edit OnUnitActiveSec in ${TIMER}, then: systemctl daemon-reload && systemctl restart kyocera-keepalive.timer"
