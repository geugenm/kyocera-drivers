#pragma once

#include <cstdint>

static constexpr std::size_t dither_table_size = 256;

void halftone_dib_to_dib(std::uint8_t* source_planes8,
                         std::uint8_t* destination_planes,
                         std::size_t   image_width,
                         std::size_t   num_rows,
                         std::int32_t  contrast,
                         std::int32_t  brightness);

void set_default_screen();

[[nodiscard]] std::uint8_t* get_current_dither_table();