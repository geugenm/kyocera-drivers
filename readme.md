<div align="center">

# kyocera_drivers

[![Package](https://github.com/e-gleba/kyocera-drivers/actions/workflows/package.yml/badge.svg)](https://github.com/e-gleba/kyocera-drivers/actions/workflows/package.yml)
[![Release](https://img.shields.io/github/v/release/e-gleba/kyocera-drivers)](https://github.com/e-gleba/kyocera-drivers/releases/latest)
[![Platform](https://img.shields.io/badge/platform-Linux%20x86__64%20%7C%20ARM-blue)](https://github.com/e-gleba/kyocera-drivers)
[![Field tested](https://img.shields.io/badge/field%20tested-FS--1020MFP-brightgreen)](https://github.com/e-gleba/kyocera-drivers)

[![▶ Run Package](https://img.shields.io/badge/▶_Run-Package-2088FF?logo=github-actions&logoColor=white)](https://github.com/e-gleba/kyocera-drivers/actions/workflows/package.yml)
[![▶ Run Release](https://img.shields.io/badge/▶_Run-Release-2088FF?logo=github-actions&logoColor=white)](https://github.com/e-gleba/kyocera-drivers/actions/workflows/release.yml)

Original PPDs and Kyocera's proprietary `rastertokpsl` filter for the 2012-era GDI printer family, packaged for modern Linux CUPS. No compiled code — CMake/CPack packaging plus a small Bash wrapper.

**Supported:** FS-1020MFP · FS-1025MFP · FS-1040 · FS-1060DN · FS-1120MFP · FS-1125MFP (GDI).

</div>

---

## Why this exists

The printers still work; their Linux support didn't survive:

- **The drivers are gone.** Kyocera moved to a universal driver and [no longer distributes](#notice) the per-model Linux packages — this repo bundles the originals and installs them where CUPS expects them.
- **The filter binary is x86-only.** Closed-source, shipped for i386/x86_64 only; ARM boards (Raspberry Pi, Orange Pi, …) run it through `qemu-i386` via the QEMU runtime from `scripts/setup.sh`.
- **It crashes on non-ASCII job titles** (e.g. Cyrillic): the binary aborts mid-job and the printer hangs in "Processing" until a power cycle. The wrapper strips titles to ASCII and forces `LC_ALL=C`.
- **It predates modern CUPS filter plumbing.**

All of that is bridged by one small Bash wrapper, installed as `rastertokpsl`: it picks the binary per machine, sanitizes the job title, and resolves filters relative to its own location.

## Quick start

### One-shot (from a checkout)

[`scripts/setup.sh`](scripts/setup.sh) does everything: prerequisites (incl. the ARM QEMU runtime), install, CUPS restart, then a non-printing verification:

```bash
git clone https://github.com/e-gleba/kyocera-drivers.git
cd kyocera-drivers
sudo bash scripts/setup.sh
```

Only `cmake` and `ninja-build` are needed beyond the CUPS stack — nothing is compiled.

### Release packages

[e-gleba/kyocera-drivers/releases](https://github.com/e-gleba/kyocera-drivers/releases/latest) carry `.deb`, `.rpm` and `.tar.gz`:

```bash
sudo dpkg -i kyocera_drivers-*-Linux.deb           # Debian / Ubuntu
sudo rpm -i kyocera_drivers-*-Linux.rpm            # Fedora / ALT Linux
sudo tar -C / -xzf kyocera_drivers-*-Linux.tar.gz  # any distro
```

### Manual from source

```bash
cmake --preset default
sudo cmake --install build/default --prefix=/usr
sudo systemctl restart cups
```

Then add the printer at <http://localhost:631> and pick the PPD matching your exact model.

## Verify without printing

[`scripts/verify.sh`](scripts/verify.sh) runs a sample page through the real chain — PPD raster format → `rastertokpsl` → KPSL — and asserts the output. Nothing is sent to the printer:

```bash
bash scripts/verify.sh                          # from a checkout
bash /usr/share/doc/kyocera_drivers/verify.sh   # from a release package
```

It ends with e.g. `OK: armv7l produced 3 KB of KPSL — nothing sent to the printer`. The same check runs in a throwaway container — the image builds, installs into a clean Debian, verifies itself:

```bash
docker build -t kyocera-drivers-test .
```

## Demo

![Quickstart: configure, build, install, wrapper tests](.github/casts/quickstart.gif)

![Verify: PPD raster through rastertokpsl, nothing printed](.github/casts/verify.gif)

Recorded with asciinema — replay or re-record via [.github/casts](.github/casts/readme.md).

## ARM boards (Raspberry Pi, Orange Pi, …)

`scripts/setup.sh` covers it; under the hood it installs the QEMU i386 runtime:

```bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install qemu-user-static libc6:i386 libcups2:i386 libcupsimage2:i386
sudo ln -sf /lib/i386-linux-gnu/ld-linux.so.2 /lib/ld-linux.so.2
```

aarch64 boards use the same commands.

## The print server

Daily driver: an **Orange Pi PC** (Allwinner H3, armv7l) on **Armbian 26.11** (Debian 13, kernel 6.18). The FS-1020MFP is USB-only, so the Pi is the CUPS server and LAN clients reach it over the network. A page rasterizes in ~2–3 s.

```mermaid
flowchart LR
    clients[LAN clients<br>PCs · Wi-Fi] --> sw[Switch / router]
    sw -->|IPP :631| pi[Orange Pi PC<br>CUPS · Armbian]
    pi -->|USB| pr[Kyocera FS-1020MFP]
```

| | |
|---|---|
| Server | `orangepipc` — `ssh egleba@192.168.1.28`, Armbian 26.11 |
| Queue | `FS-1020MFP` at `http://orangepipc.local:631/printers/FS-1020MFP` |
| Linux clients | [Connect a Linux client](#connect-a-linux-client-kde) |
| Windows clients | [Connect a Windows client](#connect-a-windows-client) |
| It won't wake | [Printer sleep](#printer-sleep) |

## Connect a Linux client (KDE)

System Settings → **Printers → Add → Network** — the queue announces itself over mDNS as *FS-1020MFP @ orangepipc*; pick it and the driver configures itself (IPP Everywhere). From the shell, same result:

```bash
sudo lpadmin -p FS-1020MFP -E -v ipp://orangepipc.local:631/printers/FS-1020MFP -m everywhere
sudo lpadmin -d FS-1020MFP
```

- The client passes documents through unchanged (`-m everywhere` imports the real capabilities); the Pi runs the whole filter chain — that is what the [wrapper](#how-it-works) is for.
- Use the **direct `ipp://` URI** — `implicitclass://` proxies from `cups-browsed` are known to swallow jobs.
- Browsers snapshot the printer list at startup — restart Chromium/Firefox after adding the queue.

## Connect a Windows client

The Kyocera **client tool** is the official Windows package for this printer family (Windows only — there is no Linux equivalent, and the Pi needs none):

1. [The downloads page](https://www.kyoceradocumentsolutions.ru/ru/support/downloads.name-L3J1L3J1L21mcC9UQVNLQUxGQTIwMjE=.html) generates a personal link — grab the client tool it offers.
2. Install it once on the Windows machine.
3. Open `\\orangepipc` in Explorer and double-click `FS-1020MFP`; Windows asks for a driver. The Pi renders everything, so even the built-in **Microsoft PS Class Driver** prints fine — the client tool's real value here is its device settings, see the next section.

## Printer sleep

The 2012 firmware drops its USB link after a long idle and can look completely dead afterwards — once it needed a physical power cycle to come back. Worth knowing before blaming the driver:

- **No host-side fix exists.** Everything was tried from the Pi and retired — PJL queries, CUPS handshakes, USB re-enumeration, even a two-minute USB poke daemon. Not one of them kept an idle device awake (all of it lives in the git history).
- **The fix lives on the device.** With the client tool installed (for [Windows clients](#connect-a-windows-client)), select *Kyocera FS-1020MFP GX* → **Конфигурация (Configuration)** → device parameters → set the **sleep timer to the maximum** (up to ~1 week). One-time, and it helps every OS at once.

![Kyocera Client Tool — the Конфигурация tile holds the device's sleep timer](.github/kyocera_client_where_to_click.jpg)

- Until then: jobs simply wait — the Pi's queue never parks (`retry-job`) and prints the moment the device wakes. A panel-button press also wakes it.

## Uninstall

```bash
sudo xargs rm -f < build/default/install_manifest.txt   # installed from source
sudo dpkg -r kyocera_drivers                            # installed from .deb
sudo rpm -e kyocera_drivers                             # installed from .rpm
```

## How it works

```mermaid
flowchart LR
    A[Application<br>PDF / PS / image] --> B[CUPS cupsd]
    B --> C[pdftopdf / gstoraster]
    C -->|CUPS raster, 600 dpi gray| D[rastertokpsl wrapper]
    D -->|x86: native · ARM: qemu-i386| E[Kyocera's filter]
    E -->|KPSL byte stream| F[USB / network backend]
    F --> G[Printer]
```

## Troubleshooting

| Symptom | Fix |
|---|---|
| `cmake: command not found`, or `No such preset "default"` | Install the tools: `sudo apt install cmake ninja-build` — CMake ≥ 3.31 is required. |
| `cupsfilter: command not found` | CUPS stack missing: `sudo apt install cups cups-filters ghostscript` (what `setup.sh` installs). |
| Remote clients can't reach `http://192.168.1.28:631` | VPN active on the client or the Pi — disable it; it must not route the LAN. |
| Filter stops with status 127, or `No such file or directory` in `error_log` | Stale/partial install. `sudo rm -f /usr/lib/cups/filter/rastertokpsl* /usr/local/lib/cups/filter/rastertokpsl*` and reinstall. |
| `wrapper.sh: QEMU i386 runtime incomplete` | ARM host missing the QEMU prerequisites — run `scripts/setup.sh`. |
| `wrapper.sh: unsupported architecture` | Host outside x86/ARM — open an issue with `uname -m`. |
| Filter errors at runtime | `sudo tail -f /var/log/cups/error_log`; add `sudo cupsctl --debug-logging` for full detail. |
| Wrong page size or orientation | Selected PPD does not match the exact printer model. |
| Jobs queue forever on a sleeping server | See [Printer sleep](#printer-sleep). |

## Known upstream problems

These live in the external stack (Kyocera's binary, CUPS defaults, libcupsfilters) — **not** in this packaging layer. Enable debug logging first; the log names the exact filter that fails:

```bash
sudo cupsctl --debug-logging && sudo systemctl restart cups
sudo tail -F /var/log/cups/error_log | grep --line-buffered -E '\[Job'
```

### 1. Printer hangs mid-job until a power cycle — non-ASCII job titles

Kyocera's binary aborts (SIGABRT/SIGSEGV) when the title or username contains non-ASCII bytes (e.g. Cyrillic on `ru_RU.UTF-8`); the printer receives a partial KPSL stream and waits forever. Windows is unaffected (different driver codebase).

```bash
lp -d <queue> -t "тест" /usr/share/cups/data/testprint    # reproduce
sudo grep -E 'crashed|signal' /var/log/cups/error_log      # confirm
```

**Mitigated by the bundled wrapper**: titles are stripped to ASCII and `LC_ALL=C` is forced, so the crashing bytes never reach the binary. The bug itself is Kyocera's.

References: [OpenPrinting/cups#966](https://github.com/OpenPrinting/cups/issues/966) · [Arch forums — FS-1061DN filter crash](https://bbs.archlinux.org/viewtopic.php?id=272961)

### 2. Queue shows "stopped" after any failed job

CUPS' default `stop-printer` error policy halts the whole queue on one failed filter; later jobs pile up behind it.

```bash
lpstat -p <queue>                                            # check
sudo lpadmin -p <queue> -o printer-error-policy=abort-job    # fix
```

### 3. Every job fails with "Unexpected page count"

libcupsfilters 2.2.1 (pdfio-based page counting) misreads its own intermediate PDF and aborts **every** job — affecting all `*cupsFilter` drivers, not just Kyocera. Fixed upstream by [libcupsfilters PR #167](https://github.com/OpenPrinting/libcupsfilters/pull/167).

```bash
sudo grep -E 'Missing Root object|Unexpected page count' /var/log/cups/error_log
```

**Fix:** update libcupsfilters past the fix, or pin 2.1.1 (qpdf-based).

---

## Notice

Kyocera no longer maintains legacy per-model Linux drivers ([universal print](https://www.kyoceradocumentsolutions.com/support/universal_print/), [global portal](https://global.kyocera.com/support/download/)). The PPDs in `ppd/English/` come from the [OpenPrinting archive](https://www.openprinting.org/download/PPD/Kyocera/en/); automated fetching is disabled by default (`DOWNLOAD_PPDS=OFF`).

---

<div align="center">

[CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) · [CPack](https://cmake.org/cmake/help/latest/module/CPack.html) · [SDB: Custom CUPS Filters](https://en.opensuse.org/SDB:Using_Your_Own_Filters_to_Print_with_CUPS)

GPL-3.0 — see [license](license).

</div>
