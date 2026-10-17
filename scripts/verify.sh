#!/usr/bin/env bash
# Non-printing check that the installed filter chain works end to end:
# PPD raster format -> rastertokpsl -> KPSL. Safe to run at any time;
# nothing is ever sent to the printer.
#
# Usage: bash scripts/verify.sh [ppd-file] [document]
#   FILTER=<path>   filter to test (default: /usr/lib/cups/filter/rastertokpsl)
set -euo pipefail

ppd="${1:-/usr/share/cups/model/Kyocera/English/Kyocera_FS-1020MFPGDI.ppd}"
filter="${FILTER:-/usr/lib/cups/filter/rastertokpsl}"

cupsfilter="$(command -v cupsfilter || true)"
[ -n "$cupsfilter" ] || cupsfilter=/usr/sbin/cupsfilter
if [ ! -x "$cupsfilter" ]; then
  echo "error: cupsfilter not found — install cups (Debian: apt install cups)" >&2
  exit 1
fi
if [ ! -f "$ppd" ]; then
  echo "error: PPD not found: $ppd — install first: sudo bash scripts/setup.sh" >&2
  exit 1
fi
if [ ! -x "$filter" ]; then
  echo "error: filter not found: $filter — install first: sudo bash scripts/setup.sh" >&2
  exit 1
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

doc="${2:-}"
if [ -z "$doc" ]; then
  doc="$tmp/doc.ps"
  printf '%%!PS\n/Helvetica findfont 14 scalefont setfont\n72 760 moveto (rastertokpsl verification page) show\nshowpage\n' > "$doc"
fi

echo "==> rasterizing $(basename "$doc") with $(basename "$ppd")"
"$cupsfilter" -p "$ppd" -m application/vnd.cups-raster "$doc" > "$tmp/job.raster"

echo "==> converting to KPSL with $filter"
"$filter" "$tmp/job.raster" "${USER:-verify}" verify 1 "PageSize=A4" \
  < "$tmp/job.raster" > "$tmp/job.kpsl"

size=$(wc -c < "$tmp/job.kpsl")
magic=$(head -c 4 "$tmp/job.kpsl")
if [ "$magic" != LSPK ] || [ "$size" -lt 1024 ]; then
  echo "error: no valid KPSL output (magic '$magic', $size bytes)" >&2
  exit 1
fi
echo "OK: $(uname -m) produced $((size / 1024)) KB of KPSL — nothing sent to the printer"
