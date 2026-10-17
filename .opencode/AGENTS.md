# kyocera_drivers

Bash + CMake packaging of Kyocera's legacy GDI printer drivers (original
PPDs + closed-source filter binaries) for modern Linux CUPS. No compiled
code. Think first. Minimal diff. Verify with `scripts/verify.sh` (never prints).

## AI files

- `.opencode/AGENTS.md` (this file) owns behavior.
- `.opencode/REVIEW.md` owns mechanical review shape.
- `.opencode/opencode.jsonc` owns config: instructions + watcher ignores.
- `.agents/shared/` owns workflow memory: `project_context.md`, `test_loop.md`.
- `.agents/skills/` is vendored third-party skills via
  `.agents/install_skills.cmake` + pinned `skills-lock.json`.

## Layout

- `CMakeLists.txt` — packaging only: install rules, CMake package config, CPack. No build targets.
- `proprietary/` — `wrapper.sh` (installed as `rastertokpsl`) + `rastertokpsl_x86` / `rastertokpsl_amd64`; never edit the binaries.
- `ppd/English/` — original Kyocera PPDs (OpenPrinting archive copies), installed to `share/cups/model`.
- `scripts/setup.sh` — one-shot: prerequisites, build, install, CUPS restart, verify. On ARM also installs the QEMU i386 runtime.
- `scripts/verify.sh` — non-printing end-to-end check (raster → KPSL magic/size). Installed into the package docdir.
- `scripts/test-wrapper.sh` — dispatch tests for the wrapper (stub binaries, fake `uname`).
- `.github/workflows/` — `package.yml`, `release.yml`.

## Wrapper (`proprietary/wrapper.sh`)

- Dispatch: x86_64 → native `rastertokpsl_amd64`; i386 → native `rastertokpsl_x86`; ARM → `qemu-i386` running the x86 binary; anything else → clear error. No other fallbacks.
- ARM preflight: `qemu-i386`/`-static` in PATH, `/lib/ld-linux.so.2`, i386 `libcups.so.2` + `libcupsimage.so.2`. Error text points to `scripts/setup.sh`.
- Always `LC_ALL=C` + ASCII-only job title (`tr -cd '[:alnum:]'`) — non-ASCII titles crash Kyocera's binary.
- Resolve filters from the wrapper's own directory, never from a baked-in prefix. CUPS arg order: file, user, title, copies, options.
- Bash strict mode everywhere; `bash -n` + `scripts/test-wrapper.sh` before commit.

## CMake

- Packaging only. `KYOCERA_*` cache vars; host arch from `CMAKE_SYSTEM_PROCESSOR` (`KYOCERA_X86_64` / `KYOCERA_X86_32`).
- Install rules: wrapper everywhere; x86 binary everywhere (needed for QEMU on ARM); amd64 x86_64-only; `verify.sh` into the docdir.
- CPack arch mapping derives from the host arch — never hardcode (armv7l → `armhf`/`armv7hl`, aarch64 → `arm64`/`aarch64`).
- Every install-rule change must be checked on x86_64 **and** ARM-simulated (`-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7l`; a bare `CMAKE_SYSTEM_PROCESSOR` override is ignored by native detection).

## Verify loop

1. `bash -n` on every touched script; `scripts/test-wrapper.sh`.
2. Packaging: `cmake -S . -B /tmp/kd -G Ninja` + `cmake --install /tmp/kd --prefix /tmp/kd-out`; check `lib/cups/filter/`.
3. Test box (armv7l Orange Pi, SSH): `git pull`, `sudo bash scripts/setup.sh`, then `bash scripts/verify.sh`.
4. **Never print** — no `lp`/`lpr` unless the user explicitly asks for paper. `scripts/verify.sh` covers everything up to the backend.

## Text files

- LF, UTF-8, no BOM. `readme.md` stays compact (~8 KB); no unlinked badges.
- No `rastertokpsl-re` references — reverse-engineered replacement filters are deliberately out of scope; this project wraps Kyocera's binary.

## Commits

- `feat:` / `fix:` / `docs:` / `chore:`; `[ai] ` prefix only when every change is AI-workflow files (`.opencode/`, `.agents/`, root `AGENTS.md`).
- One concern per commit; push `master` directly (this repo's workflow). Never force-push.

## Agent rules

1. Unclear = ask, not guess. State assumptions.
2. Simpler wins. This repo's value is packaging — push back on bloat.
3. Touch only request scope. Match sibling style; mention unrelated dead code, never delete it.
4. Secrets (SSH password, tokens): never echo, commit, or log. Ask-pass files under `/tmp` get deleted right after use.
