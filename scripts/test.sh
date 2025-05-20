#!/bin/bash
set -e

PPD="/etc/cups/ppd/FS-1020MFP.ppd" # Path to your Kyocera PPD
PDF="test.pdf"                     # Test document
RASTER="test_raster.ras"
OUTPUT="output.kpsl"

# Generate raster data with correct CUPS-style arguments
/usr/lib/cups/filter/pdftoraster 1 user "Test Job" 1 "PageSize=A4" "$PDF" >"$RASTER"

# Correct: Pass all 5 required args to wrapper.sh
sh wrapper.sh "$RASTER" user "Test Job" 1 "PageSize=A4" >"$OUTPUT"

echo "KPSL output saved to $OUTPUT"
