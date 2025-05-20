#!/usr/bin/env bash

set -euo pipefail

download_url="https://www.kyoceradocumentsolutions.eu/content/download-center/eu/drivers/all/LinuxDrv_1_1203_FS_1x2xMFP_zip.download.zip"
source_dir=".fetched"
system_arch="$(getconf LONG_BIT)"
target_dir="${source_dir}/Linux/${system_arch}bit/Global"
temp_dir="$(mktemp -d)"

trap 'rm -rf "${temp_dir}"' EXIT

printf 'detected system architecture (long_bit): %s-bit\n' "${system_arch}"

printf 'downloading kyocera drivers...\n'
wget -q -O "${temp_dir}/archive.zip" "${download_url}" || {
  printf 'error: download failed\n' >&2
  exit 1
}

mkdir -p "${source_dir}" || {
  printf 'error: could not create target directory %s\n' "${source_dir}" >&2
  exit 1
}

printf 'extracting driver archive...\n'
unzip -q "${temp_dir}/archive.zip" -d "${source_dir}"

find "${target_dir}" -type f -name '*.tar.gz' -exec sh -c '
    for archive; do
        mkdir -p ./ppd
        tar -xf "$archive" -C ./ppd --wildcards "*.ppd"
    done
' _ {} +

printf 'kyocera drivers extracted successfully to: %s\n' "${target_dir}"