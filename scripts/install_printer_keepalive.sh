#!/usr/bin/env bash
# install_printer_keepalive.sh — install the Kyocera "printer not turning
# off" daemon on this host: a USB keepalive (oneshot service + timer).
#
# Installs:
#   /usr/local/sbin/kyocera-keepalive              the probe itself
#   /etc/systemd/system/kyocera-keepalive.service  oneshot unit
#   /etc/systemd/system/kyocera-keepalive.timer    every KEEPALIVE_INTERVAL
#   /etc/default/kyocera-keepalive                 device/queue/read settings
#   /etc/modules-load.d/kyocera-keepalive.conf     loads usblp at boot
#   /etc/udev/rules.d/69-kyocera-keepalive.rules   never autosuspend printer
#
# Usage: sudo ./install_printer_keepalive.sh
# Optional interval: KEEPALIVE_INTERVAL=5min sudo -E ./install_printer_keepalive.sh

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
# kyocera-keepalive — nudge the Kyocera FS-1020MFP over USB so an idle
# printer never ends up in the un-wakeable deep-sleep state.
#
# Sends one empty PJL JOB envelope per run: the exact UEL/@PJL JOB/EOJ
# framing the Kyocera KPSL driver wraps around every print job, but with
# no raster payload — zero pages to printable, yet the firmware counts it
# as print activity and resets its idle timer.
#
# If the poke fails, the run scans the usb bus (lpinfo) for the printer
# and, when configured, force-re-enumerates it host-side (authorized
# 0 -> 1) before retrying the poke once.
set -euo pipefail

device="${KYOCERA_KEEPALIVE_DEVICE:-/dev/usb/lp0}"
queue="${KYOCERA_KEEPALIVE_QUEUE:-FS-1020MFP}"
want_read="${KYOCERA_KEEPALIVE_READ:-1}"
auto_recover="${KYOCERA_KEEPALIVE_AUTO_RECOVER:-1}"

probe_bytes=$'\x1b%-12345X@PJL\r\n@PJL JOB NAME = "keepalive"\r\n@PJL INFO STATUS\r\n@PJL RDYMSG DISPLAY = ""\r\n@PJL EOJ\r\n\x1b%-12345X'

if [ ! -e "${device}" ]; then
    fallback="$(ls /dev/usb/lp* 2>/dev/null | head -n1 || true)"
    if [ -n "${fallback}" ]; then
        device="${fallback}"
    fi
fi
if [ ! -w "${device}" ]; then
    echo "${device}: no accessible usb printer node (usblp loaded? printer plugged in?)" >&2
    exit 1
fi

# A live CUPS job already keeps the device awake; never interleave with it.
pending="$(lpstat -o "${queue}" 2>/dev/null || true)"
if [ -n "${pending}" ]; then
    echo "${queue}: jobs in queue, skipping this cycle"
    exit 0
fi

# udevadm reports syspaths without the /sys prefix; walk up to the usb
# device dir that carries idVendor/idProduct.
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

# One empty-job poke; success requires the usb write to complete.
poke() {
    local dev="$1" reply=""
    if ! exec 3<>"${dev}"; then
        echo "${dev}: cannot open usb printer node (backend busy?)" >&2
        return 1
    fi
    ok=0
    if timeout 8s bash -c 'printf "%s" "$1" >&3 2>/dev/null' _ "${probe_bytes}"; then
        ok=1
    fi
    if [ "${want_read}" = "1" ]; then
        reply="$(timeout 2s dd if=/dev/fd/3 bs=256 count=4 2>/dev/null || true)"
        reply="$(printf '%s' "${reply}" | tr -dc ' -~' | head -c 120)"
    fi
    exec 3>&-
    if [ "${ok}" = "1" ]; then
        if [ -n "${reply}" ]; then
            echo "keepalive ok: empty PJL job envelope on ${dev}, printer answered: ${reply}"
        else
            echo "keepalive ok: empty PJL job envelope on ${dev}"
        fi
        return 0
    fi
    echo "${dev}: usb write failed" >&2
    return 1
}

# What does the usb bus itself say about the printer right now? (diagnostic
# scan; also wakes a lightly wedged device by reading its device id)
scan() {
    lpinfo -v 2>/dev/null | grep -i kyocera || echo "scan: Kyocera not enumerated on the usb bus right now" >&2
}

# Host-side re-enumeration of a wedged printer port (authorized 0 -> 1).
recover() {
    local authorized
    authorized="$(usb_device_dir "$1" 2>/dev/null || true)/authorized"
    if [ ! -e "${authorized}" ]; then
        return 1
    fi
    echo "attempting host-side usb re-enumeration of the printer port" >&2
    echo 0 >"${authorized}" || true
    sleep 2
    echo 1 >"${authorized}" || true
    sleep 3
    return 0
}

if poke "${device}"; then
    exit 0
fi

# Failure path: rescan the bus, optionally re-enumerate the port and
# retry the poke once with a freshly bound node.
echo "rescanning usb bus for the printer" >&2
scan

fresh="$(ls /dev/usb/lp* 2>/dev/null | head -n1 || true)"
[ -n "${fresh}" ] && device="${fresh}"

if [ "${auto_recover}" = "1" ] && recover "${device}" && [ -e "${device}" ]; then
    if poke "${device}"; then
        echo "keepalive recovered after usb re-enumeration"
        exit 0
    fi
fi

echo "printer is not responding; will retry next cycle" >&2
exit 1
EOF_KEEPALIVE
chmod 755 "${BIN}"
bash -n "${BIN}"

cat >"${DEFAULT_FILE}" <<EOF_DEFAULTS
# Settings read by kyocera-keepalive.service before each run.
KYOCERA_KEEPALIVE_DEVICE=${lp_device}
KYOCERA_KEEPALIVE_QUEUE=${queue}
# 1 = read the PJL status reply (costs a couple of seconds), 0 = write-only
KYOCERA_KEEPALIVE_READ=1
# 1 = on usb write failure, re-scan the bus and re-enumerate the port, 0 = off
KYOCERA_KEEPALIVE_AUTO_RECOVER=1
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
systemctl enable --now kyocera-keepalive.timer
systemctl start kyocera-keepalive.service || \
    echo "warning: first keepalive run failed; see: journalctl -u kyocera-keepalive -n 20" >&2

echo "printer keepalive installed:"
echo "  printer: ${lp_device} (usb ${usb_vendor:-?}:${usb_product:-?})"
echo "  queue:   ${queue}"
echo "  probe:   every ${interval} (empty PJL job envelope, nothing printable)"
echo "  remove:  sudo $(dirname "$0")/uninstall_printer_keepalive.sh"
echo "checks:"
echo "  systemctl list-timers kyocera-keepalive.timer"
echo "  journalctl -u kyocera-keepalive -n 20"
echo "config: ${DEFAULT_FILE} (device/queue/reply-reading; applied on next run)"
echo "interval: edit OnUnitActiveSec in ${TIMER}, then: systemctl daemon-reload && systemctl restart kyocera-keepalive.timer"
