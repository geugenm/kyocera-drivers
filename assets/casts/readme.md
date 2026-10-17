# Terminal sessions

| Session | Replay | GIF |
| --- | --- | --- |
| Configure, install (to `/tmp/kyocera-demo`) + wrapper tests | `quickstart.cast` | `quickstart.gif` |
| Non-printing verification: PPD raster → `rastertokpsl` → KPSL | `verify.cast` | `verify.gif` |

GIFs are embedded in the root readme; `.cast` files replay with `asciinema play`.

## How to recreate

```bash
sudo dnf install asciinema        # recorder (Debian/Ubuntu: sudo apt install asciinema)
cargo install --git https://github.com/asciinema/agg  # .cast -> .gif

cmake -P assets/casts/record.cmake  # re-records all, re-renders all GIFs
```

All settings (theme, font size, idle cap, fps) live in the CONFIG block at the
top of `record.cmake` — edit and re-run. Failures abort via FATAL_ERROR, so a
finished run means all sessions exited 0.
