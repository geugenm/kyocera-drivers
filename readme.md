# Kyocera Reverse Engineered `rastertokpsl` – Modern CMake Project

## Overview

This repository delivers a fully open, reverse-engineered Kyocera PPD and filter solution for Linux printing environments. It includes:

- Modern C++20/C17 source for `rastertokpsl` (Kyocera raster filter)
- Automated CMake build and install system
- PPD files, filter binaries, and installation scripts
- Reverse engineering resources (Ghidra projects, reconstructed code)

**Supported:**

- x86_64 Linux
- CUPS print servers (requires `libcups`/`cups-devel`)

## Supported Models

This driver package provides PPDs and filter support for the following Kyocera printers:

- Kyocera FS-1020MFP GDI
- Kyocera FS-1025MFP GDI
- Kyocera FS-1040 GDI
- Kyocera FS-1060DN GDI
- Kyocera FS-1120MFP GDI
- Kyocera FS-1125MFP GDI

**Tested:**

- Fully tested and verified with Kyocera FS-1020MFP.

---

## Features

- **Modern CMake**: Out-of-the-box build, install, and test via CMake presets
- **Multi-language PPDs**: User-selectable driver language
- **Automated Install/Uninstall**: Clean system integration and removal
- **Reverse Engineering Resources**: Ghidra projects and annotated C/C++ sources

---

## Quick Start

### Prerequisites

- Fedora, Ubuntu, or any modern Linux with CUPS
- `cmake` (≥3.16 recommended)
- `make` or Ninja
- `g++` (C++20) and `gcc` (C17)
- CUPS development headers:  
  Fedora: `sudo dnf install cups-devel`  
  Ubuntu: `sudo apt install libcups2-dev`

### Build & Install

```sh
# 1. Clone the repo
git clone https://github.com/geugenm/kyocera-drivers.git
cd kyocera-drivers

# 2. Create a build directory
mkdir build && cd build

# 3. Configure the project (release build, custom install prefix optional)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Build everything
cmake --build . --parallel

# 5. Install (requires root for system-wide)
sudo cmake --install build/release --prefix=/usr
```

#### Custom Install Location

```sh
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build . --parallel
cmake --install .
```

### Components

To install only runtime:

```sh
cmake --install build --component runtime
```

To install only devel:

```sh
cmake --install build --component devel
```

### Uninstall

CMake tracks all installed files in `install_manifest.txt`. To uninstall:

```sh
cd build
sudo xargs rm < install_manifest.txt
```

Or use the provided uninstall target if available:

```sh
sudo cmake --build . --target uninstall
```

_(If not present, see [CMake uninstall recipe] for adding this target.)_

---

## Usage

After installation, CUPS will recognize the new Kyocera PPDs and filters.  
Add a printer via CUPS web UI or `lpadmin`, selecting the installed Kyocera driver.

To test the filter directly:

```sh
build/rastertokpsl <args>
```

See `--help` for argument details.

---

## Troubleshooting

- Ensure `/usr/share/cups/model/Kyocera` and `/usr/lib/cups/filter` are writable for install.
- Missing dependencies? Install `cups-devel` and CMake.
- Use `cmake --build . --verbose` for detailed build logs.
- For filter debugging, check `/var/log/cups/error_log`.

---

## Reverse Engineering Resources

- **Ghidra Project Files**:  
  Deep-dive into Kyocera’s original binary with SRE tooling.
- **Reconstructed Sources**:  
  C/C++ code derived from reverse engineering, annotated for clarity.

**Use cases:**

- Custom print workflow development
- Security research and vulnerability analysis
- Educational insight into Kyocera’s print pipeline

---

## License

This project is licensed under the GNU GENERAL PUBLIC LICENSE v2. See [LICENSE](LICENSE).

---

## References

- [CMake Install & Uninstall Guide](https://cmake.org/cmake/help/latest/guide/tutorial/Installing%20and%20Testing.html)
- [CUPS Filter Integration](https://en.opensuse.org/SDB:Using_Your_Own_Filters_to_Print_with_CUPS)
- [Ghidra Reverse Engineering](https://ghidra-sre.org/)
- [Original Repo](https://github.com/sv99/rastertokpsl-re?tab=readme-ov-file)

---
