# Contributing to kyocera_drivers

Thank you for considering a contribution. This document explains how to propose changes, report bugs, and submit pull requests.

## Development Setup

This is a packaging repository (Bash + CMake, nothing compiled):

```bash
git clone https://github.com/e-gleba/kyocera-drivers.git
cd kyocera-drivers
sudo bash scripts/setup.sh        # prerequisites, build, install, verify
```

Verify changes before committing:

```bash
bash -n scripts/*.sh proprietary/wrapper.sh
bash scripts/test_wrapper.sh      # wrapper dispatch tests
bash scripts/verify.sh            # end-to-end check, never prints
```

## Branch Naming

Use the following prefixes so CI and changelogs can categorize your work automatically:

| Prefix | Purpose |
|---|---|
| `feat/` | New feature or model support |
| `fix/` | Bug fix |
| `docs/` | Documentation or readme changes |
| `build/` | CMake, build system, or packaging |
| `test/` | Test suite changes |
| `refactor/` | Code refactoring without behavior change |

Example: `feat/add_fs_1060dn_ppd`

## Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short summary>

<body explaining what and why>

Refs: <issue or pr number>
```

Allowed types: `feat`, `fix`, `docs`, `build`, `test`, `refactor`, `chore`, `ci`

## Pull Request Checklist

Before requesting review, verify:

- [ ] The branch is rebased on the latest `master`
- [ ] `bash -n` passes on every touched script
- [ ] `bash scripts/test_wrapper.sh` passes
- [ ] `bash scripts/verify.sh` passes (never sends anything to a printer)
- [ ] `cmake --preset default && cmake --install build/default --prefix=/tmp/kd-check` installs the expected files for x86_64 and ARM-simulated builds
- [ ] Markdown files render correctly on GitHub
- [ ] Commit messages follow Conventional Commits
- [ ] The PR description explains the problem and the solution

## Code Style

- Shell: `#!/usr/bin/env bash`, `set -euo pipefail`, quoted expansions; check with `bash -n`
- CMake: format with `cmake-format` using `.cmake-format.yaml`
- Markdown: LF line endings, one H1 per file

## Reporting Bugs

Use the [bug report issue template](../issue_templates/bug_report.md). Include:

- Printer model and firmware version
- Linux distribution and CUPS version
- Install method (`scripts/setup.sh`, package, or manual)
- Exact command or CUPS job that triggered the failure
- Relevant excerpts from `/var/log/cups/error_log`

## Adding Printer Support

- Source PPDs from the [OpenPrinting archive](https://www.openprinting.org/download/PPD/Kyocera/en/) and place them in `ppd/English/`
- Note the model and where its PPD came from in the PR description
- Confirm the filter chain with `bash scripts/verify.sh <ppd>`

## Questions?

Open a [discussion](https://github.com/e-gleba/kyocera-drivers/discussions) or mention `@e-gleba` in an issue.
