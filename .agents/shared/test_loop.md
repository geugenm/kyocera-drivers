# Verify loop

Change → cheap local check → packaging check → (only if needed) test box.

1. Scripts: `bash -n <file>`; wrapper changes also run `scripts/test_wrapper.sh`.
2. Packaging: `cmake -S . -B /tmp/kd -G Ninja` then
   `cmake --install /tmp/kd --prefix /tmp/kd-out`; inspect `lib/cups/filter/`.
   - x86_64: wrapper + both binaries; `verify.sh` in the docdir.
   - ARM-simulated (`-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7l`):
     wrapper + x86 binary only, CPack labels `armhf`/`armv7hl`.
   - Clean container: `docker build -t kyocera-drivers-test .` runs the
     install + `verify.sh` on a pristine Debian.
3. Test box: SSH to the Orange Pi (ask the user for access), `git pull`,
   `sudo bash scripts/setup.sh`, then `bash scripts/verify.sh` — it must end
   with `OK: … KB of KPSL`.
4. Never print (`lp`, `lpr`) unless the user explicitly asks for paper.
   `scripts/verify.sh` covers everything up to the backend.
