# Review Style Guide

Mechanical checks for review passes. `.opencode/AGENTS.md` owns rules;
this file owns procedure and shape. Conflicts resolve to `AGENTS.md`, and
this file gets fixed.

## Procedure

- Changed lines only. No drive-by edits.
- Order: secrets, destructive commands, correctness (bash/CMake), cross-arch
  impact, wording, style.
- Bash claims: run `bash -n`; flag unquoted expansions, `[ … ] && cmd`
  statements that break under `set -e`, `command -v` without a `/usr/sbin`
  fallback (regular users lack it in PATH), fragile PATH assumptions.
- CMake: install-rule changes need the x86_64 + ARM-simulated install outputs;
  CPack arch labels must stay consistent with the host-arch mapping.
- Claims need evidence: script behavior from a run, CMake behavior from a
  configure. Mark the rest `[unverified]`.
- Output `file:line` + rule + one-line fix, severity-sorted (error, warning,
  nit). End with a 3-line verdict: approve / approve-with-nits / request-changes.

## Shape

- Scripts: `#!/usr/bin/env bash`, `set -euo pipefail`, `$(…)`, quoted
  expansions, `if` over `&&` at statement level, lowercase functions,
  comments only where behavior is non-obvious.
- readme: one H1, H2 sections, tables for troubleshooting, no duplicated
  details between sections — link instead.
- No secret values anywhere, not even as examples.
