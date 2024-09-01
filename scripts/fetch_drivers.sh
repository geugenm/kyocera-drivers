#!/bin/bash

download_url="https://www.kyoceradocumentsolutions.eu/content/download-center/eu/drivers/all/LinuxDrv_1_1203_FS_1x2xMFP_zip.download.zip"
source_dir=".fetched"

system_arch=$(getconf LONG_BIT)
echo "Detected system architecture (LONG_BIT): ${system_arch}-bit"

target_dir="${source_dir}/Linux/${system_arch}bit/Global"

temp_dir=$(mktemp -d)
trap "rm -rf '${temp_dir}'" EXIT  # Cleanup temp directory on exit

echo "Downloading Kyocera drivers..."
wget -q -O "${temp_dir}/archive.zip" "$download_url"  # Download quietly (-q)

if [[ $? -ne 0 || ! -f "${temp_dir}/archive.zip" ]]; then
  echo "Error: Download failed. Please check network connectivity and URL."
  exit 1
fi

mkdir -p ${source_dir} || {
  echo "Error: Could not create target directory '${source_dir}'."
  exit 1
}

echo "Extracting driver archive..."
unzip -q "${temp_dir}/archive.zip" -d ${source_dir}  # Extract quietly (-q)

# Extract and clean up tar.gz archives within the target directory
find ${target_dir} -type f -name "*.tar.gz" -print -exec sh -c 'tar -xf {} -C ./ppd --wildcards "*.ppd"' \; # TODO: Fix hardcoded path ../ppd

echo "Kyocera drivers extracted successfully to: '${target_dir}'"
