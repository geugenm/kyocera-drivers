#!/usr/bin/env bash

cat << EOF
Kyocera PPD installer v.1.0.4

This installer contains i386 and x86_64 linux drivers
for Kyocera FS printers and multi-functional devices.
> Kyocera FS-1020MFP
EOF

install_drivers()
{
  # check if started as root
  if [ $UID != 0 ]
  then
      echo Not sufficient rights for installation.
      echo Use "sudo" or start as root.
      exit
  fi

  base_path="../ppd"

  # Get all entries (directories and files) in the current directory
  shopt -s nullglob  # Avoid listing no matches for patterns
  entries=(${base_path}/*)

  # Filter only directories for user selection
  dirs=()
  for entry in "${entries[@]}"; do
    if [[ -d "$entry" ]]; then
      dirs+=("$entry")
    fi
  done

  # Check if any directories exist
  if [[ ${#dirs[@]} -eq 0 ]]; then
    echo "No directories found in base_path=[${base_path}]."
    exit 1
  fi

  # Loop through directories and print names without path prefix
  echo "Available languages:"
  count=1
  for dir in "${dirs[@]}"; do
    # Print only the directory name (remove trailing slash and leading ../)
    dir_name="${dir##*/}"  # Use parameter expansion for efficient removal
    echo "$count. $dir_name"
    ((count++))
  done

  # User choice
  read -p "Enter the number of the driver language to choose (or 0 to exit): " choice

  # Check for valid input (0 to exit or number within directory count)
  if [[ $choice -eq 0 ]]; then
    echo "Exiting..."
    exit 0
  elif [[ $choice -lt 1 || $choice -gt $((count - 1)) ]]; then
    echo "Invalid choice. Please enter a number between 1 and $((count - 1)) or 0 to exit."
    exit 1
  fi

  # Get the chosen directory based on user input (remember indexing starts from 0)
  chosen_dir="${dirs[((choice - 1))]}"

  # Print the chosen directory (without path prefix)
  chosen_dir_name="${chosen_dir##*/}"
  echo Installing...

  source_path="${base_path}/${chosen_dir_name}"

  echo "Creating Kyocera model directory in CUPS..."
  mkdir -p /usr/share/cups/model/Kyocera

  echo "Copying PPD files for '${chosen_dir_name}'..."
  cp ${source_path}/*.ppd /usr/share/cups/model/Kyocera

  system_arch=$(getconf LONG_BIT)
  echo "Detected system architecture (LONG_BIT): ${system_arch}-bit"

  filter_script="${base_path}/rastertokpsl_${system_arch}"
  filter_script_wrapper="${base_path}/rastertokpsl_wrapper"

  echo "Copying filter scripts..."
  cp ${filter_script} /usr/lib/cups/filter/rastertokpsl-bin
  cp ${filter_script_wrapper} /usr/lib/cups/filter/rastertokpsl

  echo "Kyocera device drivers installed successfully. You may now use CUPS."
}

if [[ "$1" == "--uninstall" ]]; then
  echo "Uninstalling Kyocera PPD drivers..."

  rm -rf /usr/lib/cups/filter/{rastertokpsl-bin,rastertokpsl} \
       /usr/share/cups/model/Kyocera/Kyocera_FS-1{020MFP}GDI.ppd

  rmdir --ignore-missing /usr/share/cups/model/Kyocera

  echo "Kyocera PPD drivers uninstalled successfully."
  exit 0
fi

read -p "Install Kyocera PPD drivers? (y/N) " -r -n 1 answer
echo  # Add a newline after user input

if [[ ${answer} =~ ^[Yy]$ ]]; then
  install_drivers
else
  echo "Installation cancelled."
  exit 0
fi