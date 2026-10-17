# Project context

Legacy Kyocera GDI printer drivers, packaged for modern CUPS — continuity
notes for AI sessions. Current request + current source win over anything
written here.

## Fact sheet

- Field-test box: Orange Pi PC (armv7l, Armbian/Debian 13), CUPS queue
  `FS-1020MFP` over USB; SSH access is configured out-of-band (ask the user).
- Kyocera ships only i386/x86_64 `rastertokpsl` — no ARM build exists. On ARM
  the wrapper runs the 32-bit binary via `qemu-i386`, with Debian multiarch
  i386 `libcups*` and the `/lib/ld-linux.so.2` loader symlink.
- Printer raster format: 600 dpi 8-bit gray, produced by the PPD chain
  (`pdftopdf` → `gstoraster`). A wrong-format raster (e.g. RGB) makes the
  proprietary binary corrupt its heap and abort — always test with
  PPD-generated rasters.
- Job path: app → cupsd → pdftopdf → gstoraster → raster → `rastertokpsl`
  wrapper → [qemu] → Kyocera binary → KPSL → USB backend.
- Known crash: non-ASCII job titles abort the binary and the printer hangs
  until a power cycle. The wrapper strips titles to ASCII and sets `LC_ALL=C`.
- QEMU cost: ~2–3 s per A4 page rasterized on the Orange Pi PC.
- `rastertokpsl-re` is deliberately out of this project — do not reintroduce
  it in code, docs, or fallbacks.

## Document roles

- `readme.md` user-facing; `.opencode/AGENTS.md` behavior; `.opencode/REVIEW.md`
  review shape; `.agents/shared/` workflow memory (this file + `test_loop.md`).
