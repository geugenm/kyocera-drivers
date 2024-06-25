#!/bin/bash

[[ $EUID -ne 0 ]] && exec sudo -k -p "[sudo] Enter password: " "$0" "$@"

# Define paths for PPD files and filters (can be adjusted)
base_path="${base_path:-../ppd}"
filters_path="${filters_path:-../filters}"

display_installer_info()
{
  # Kyocera PPD installer information
  echo "Kyocera PPD installer v2.0"
  echo
  echo "This installer contains i386 and x86_64 Linux drivers"
  echo "for Kyocera FS printers and multi-functional devices."
  echo "Included devices:"

  # Function to list formatted printer names from PPD files in a directory
  list_printer_names() {
    local dir="$1"

    # Find .ppd files, use awk to format filenames, and print with "> " prefix
    find "$dir" -type f -name "*.ppd" -print0 |
      xargs -0 basename -a |
      awk -F '_' '{ gsub(/_/, " ", $0); sub(/GDI.*$/, "", $0); print "> " $0 }'
  }

  # Check if base path exists
  if [[ ! -d "$base_path" ]]; then
    echo "Error: Directory '$base_path' does not exist." >&2
    exit 1
  fi

  # List printer names from a random subdirectory
  list_printer_names "$(find "$base_path" -type d ! -path "$base_path" -print | sort -R | head -n 1)"
}

install_drivers() {
  local lang_options=()
    for lang_dir in "$base_path"/*/; do
      [[ -d "$lang_dir" ]] || continue
      lang_name="${lang_dir%/}"
      lang_name="${lang_name##*/}"
      lang_options+=("$lang_name")
    done

    PS3="Choose driver language (enter number or Ctrl+C to cancel): "
    echo ""
    select lang_name in "${lang_options[@]}"; do
      [[ -n "$lang_name" ]] && break
    done

  # Handle user cancellation
    if [[ -z "$lang_name" ]]; then
      echo "Installation cancelled." >&2
      exit 1
    fi

    lang_dir="$base_path/$lang_name/"

  # Confirm installation (without hardcoded path)
  read -r -p "Install drivers for language: '$lang_name'? [y/N] " response
  [[ "$response" =~ ^[Yy]$ ]] || { echo "Installation cancelled." >&2; exit 1; }

  # Installation steps
  echo "Installing..."
  mkdir -p /usr/share/cups/model/Kyocera &&
    cp "${lang_dir}"/*.ppd /usr/share/cups/model/Kyocera &&
    cp "${filters_path}/rastertokpsl_$(getconf LONG_BIT)" /usr/lib/cups/filter/rastertokpsl-bin &&
    cp "${filters_path}/rastertokpsl_wrapper" /usr/lib/cups/filter/rastertokpsl &&
    echo "Kyocera device drivers installed successfully. You may now use CUPS." || {
      echo "Error: Installation failed." >&2
      exit 1
    }
}

uninstall_drivers() {
  echo "Uninstalling Kyocera PPD drivers..."
  rm -rf /usr/lib/cups/filter/{rastertokpsl-bin,rastertokpsl} /usr/share/cups/model/Kyocera &&
    echo "Kyocera PPD drivers uninstalled successfully." ||
    { echo "Error: Failed to remove Kyocera driver files." >&2; return 1; }
}

display_installer_info

if [[ "$1" == "--uninstall" ]]; then
  uninstall_drivers
else
  read -p "Install Kyocera PPD drivers? [y/N] " -r -n 1 answer
  [[ "$answer" =~ ^[Yy]$ ]] && install_drivers
fi