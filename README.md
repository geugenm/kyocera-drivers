# Kyocera reverse engineered `rastertokps` (NOT WORKING ANYMORE!!!)

## Description

This repository provides tools to install reverse engineered Kyocera PPD drivers(+ their source code) on Linux systems.

**Features:**

- Supports both x86_64 architectures.
- Provides a user-friendly interface to select the driver language.
- Installs PPD files, rastertokpsl filters, and wrapper script.
- Offers an uninstall option to remove all installed components.
- Uses c++20 standards for most of operations (~60% is rewritten on c++20)

To use it you will require to install cups library (cups-dev)

## Troubleshooting

If the script encounters errors, check that the required directories (base_path, filters_path) exist and contain the correct files.

Make sure you have write permissions to the `/usr/share/cups/model/Kyocera` and `/usr/lib/cups/filter` directories.

Make sure to use cmake presets to build the project.

## Reverse Engineering `rastertokps`

This repository also includes resources for reverse engineering the rastertokps filter, a crucial component of the Kyocera printing process.

Resources:

    Ghidra Project Files: Reverse engineering analysis using Ghidra, a free and open-source software reverse engineering (SRE) framework.

    Source Code (Reconstructed): Re-implemented C/C++ code based on the analysis from Ghidra, providing insights into the filter's functionality.

These resources are invaluable for:

    Understanding Kyocera's printing pipeline: Analyze how the rastertokps filter processes data and interacts with the printer.

    Developing custom printing solutions: Gain the knowledge to create tailored printing workflows or troubleshoot specific issues.

    Security research: Identify potential vulnerabilities in the printing process.

### Disclaimer:

Reverse engineering is a complex field, and the provided resources serve as a starting point for further analysis. The accuracy and completeness of the reconstructed C code are not guaranteed.

## Uninstalling

```sudo xargs rm < install_manifest.txt```

## License

This script is released under the GNU GENERAL PUBLIC LICENSE, see [LICENSE](LICENSE)

## References

https://github.com/sv99/rastertokpsl-re?tab=readme-ov-file
