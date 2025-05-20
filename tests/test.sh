#!/usr/bin/env bash

set -euo pipefail

pdf_file="test.pdf"
raster_file="test_raster.ras"
output_file="output.kpsl"

/usr/lib/cups/filter/pdftoraster 1 user "test_job" 1 "PageSize=A4" "$pdf_file" >"$raster_file"

exec /usr/lib/cups/filter/rastertokpsl-bin 1 user "test_job" 1 "PageSize=A4" "$raster_file" >"$output_file"
