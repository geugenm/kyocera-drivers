#!/bin/bash

script_dir=$(dirname "$(readlink -f "$0")")

if [[ ! -f "CMakePresets.json" ]]; then
  echo "Error: CMakePresets.json not found." >&2
  exit 1
fi

presets=$(jq -r '.configurePresets[].name' CMakePresets.json)

if [[ -z "$presets" ]]; then
  echo "Error: No presets found in CMakePresets.json." >&2
  exit 1
fi

echo "Available presets:"
select chosen_preset in $presets; do
    if [[ -n "$chosen_preset" ]]; then
        break
    fi
    echo "Invalid choice. Please try again." >&2
done

cmake --preset "$chosen_preset" -S . -B ".build/${chosen_preset,,}"
cmake --build ".build/${chosen_preset,,}" --config "$chosen_preset"