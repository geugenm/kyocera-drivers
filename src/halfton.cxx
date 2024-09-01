#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "halfton.hxx"

std::vector<std::uint8_t> dither_table{};

std::uint32_t dither_table_width  = 0;
std::uint32_t dither_table_height = 0;
std::int32_t  dither_table_pitch  = 0;

std::uint8_t* get_current_dither_table()
{
    return dither_table.data();
}

[[nodiscard]] std::uint8_t apply_transfer_function(
    std::uint8_t        value,
    const std::int32_t& contrast,
    const std::int32_t& brightness)
{
    if (contrast != 0)
    {
        static constexpr float contrast_factor_1 = 0.000024999999F;
        static constexpr float contrast_factor_2 = 0.0074999998F;

        const float adjusted_contrast =
            contrast_factor_1 * contrast * contrast +
            contrast_factor_2 * contrast + 1.0f;

        const float adjusted_value =
            (value - 128.0f) * adjusted_contrast + 128.0f;
        value = std::clamp(adjusted_value, 0.0f, 255.0f);
    }

    if (brightness != 0)
    {
        static constexpr float brightness_factor = 0.0049999999F;

        const float adjusted_brightness = brightness_factor * brightness;
        const float adjusted_value      = value + adjusted_brightness * 255.0f;

        value = std::clamp(adjusted_value, 0.0f, 255.0f);
    }

    return value;
}

static constexpr std::array<std::uint8_t, dither_table_size>
    device_best_dither = {
        0x91, 0xB9, 0xB1, 0x89, 0x6B, 0x43, 0x4B, 0x73, 0x93, 0xBB, 0xB3, 0x8B,
        0x69, 0x41, 0x49, 0x71, 0xC1, 0xF1, 0xE9, 0xA1, 0x3B, 0x0B, 0x13, 0x5B,
        0xC3, 0xF3, 0xEB, 0xA3, 0x39, 0x09, 0x11, 0x59, 0xC9, 0xF9, 0xE1, 0xA9,
        0x33, 0x03, 0x1B, 0x53, 0xCB, 0xFB, 0xE3, 0xAB, 0x31, 0x01, 0x19, 0x51,
        0x99, 0xD9, 0xD1, 0x81, 0x63, 0x23, 0x2B, 0x7B, 0x9B, 0xDB, 0xD3, 0x83,
        0x61, 0x21, 0x29, 0x79, 0x6C, 0x44, 0x4C, 0x74, 0x94, 0xBC, 0xB4, 0x8C,
        0x6E, 0x46, 0x4E, 0x76, 0x96, 0xBE, 0xB6, 0x8E, 0x3C, 0x0C, 0x14, 0x5C,
        0xC4, 0xF4, 0xEC, 0xA4, 0x3E, 0x0E, 0x16, 0x5E, 0xC6, 0xF6, 0xEE, 0xA6,
        0x34, 0x04, 0x1C, 0x54, 0xCC, 0xFC, 0xE4, 0xAC, 0x36, 0x06, 0x1E, 0x56,
        0xCE, 0xFE, 0xE6, 0xAE, 0x64, 0x24, 0x2C, 0x7C, 0x9C, 0xDC, 0xD4, 0x84,
        0x66, 0x26, 0x2E, 0x7E, 0x9E, 0xDE, 0xD6, 0x86, 0x92, 0xBA, 0xB2, 0x8A,
        0x68, 0x40, 0x48, 0x70, 0x90, 0xB8, 0xB0, 0x88, 0x6A, 0x42, 0x4A, 0x72,
        0xC2, 0xF2, 0xEA, 0xA2, 0x38, 0x08, 0x10, 0x58, 0xC0, 0xF0, 0xE8, 0xA0,
        0x3A, 0x0A, 0x12, 0x5A, 0xCA, 0xFA, 0xE2, 0xAA, 0x30, 0x00, 0x18, 0x50,
        0xC8, 0xF8, 0xE0, 0xA8, 0x32, 0x02, 0x1A, 0x52, 0x9A, 0xDA, 0xD2, 0x82,
        0x60, 0x20, 0x28, 0x78, 0x98, 0xD8, 0xD0, 0x80, 0x62, 0x22, 0x2A, 0x7A,
        0x6F, 0x47, 0x4F, 0x77, 0x97, 0xBF, 0xB7, 0x8F, 0x6D, 0x45, 0x4D, 0x75,
        0x95, 0xBD, 0xB5, 0x8D, 0x3F, 0x0F, 0x17, 0x5F, 0xC7, 0xF7, 0xEF, 0xA7,
        0x3D, 0x0D, 0x15, 0x5D, 0xC5, 0xF5, 0xED, 0xA5, 0x37, 0x07, 0x1F, 0x57,
        0xCF, 0xFE, 0xE7, 0xAF, 0x35, 0x05, 0x1D, 0x55, 0xCD, 0xFD, 0xE5, 0xAD,
        0x67, 0x27, 0x2F, 0x7F, 0x9F, 0xDF, 0xD7, 0x87, 0x65, 0x25, 0x2D, 0x7D,
        0x9D, 0xDD, 0xD5, 0x85
    };

void set_dither_gray_table(const std::int8_t*   input_table,
                           const std::uint32_t& width,
                           const std::uint32_t& height)
{
    dither_table.clear();

    dither_table_width  = width;
    dither_table_height = height;

    const std::int32_t padding = (width & 7) ? 7 : 0;

    dither_table_pitch           = dither_table_width + padding;
    const std::size_t table_size = dither_table_pitch * dither_table_height;

    dither_table = std::vector<std::uint8_t>(table_size, 0);

    for (std::size_t row = 0; row < dither_table_height; ++row)
    {
        for (std::size_t col = 0; col < dither_table_pitch; ++col)
        {
            const std::size_t destination_index =
                col + row * dither_table_pitch;

            const std::size_t source_col = col % dither_table_width;

            dither_table[destination_index] = static_cast<std::uint8_t>(
                -1 - input_table[row * dither_table_width + source_col]);
        }
    }
}

[[nodiscard]] constexpr std::size_t get_line_bytes(
    const std::int32_t& width, const std::int32_t& multiplier)
{
    const std::int32_t totalBytes   = multiplier * width;
    const std::int32_t alignedBytes = (totalBytes + 31) & ~31;
    return alignedBytes >> 3;
}

[[nodiscard]] std::uint8_t apply_dithering_and_transfer(
    const std::uint8_t* transfer_table,
    const std::uint8_t* source_row_pointer,
    const std::uint8_t* dither_table_column)
{
    std::uint8_t result = 0;

    for (std::size_t i = 0; i < 8; ++i)
    {
        const std::uint8_t transfer_value =
            transfer_table[source_row_pointer[i]];
        const std::uint8_t dither_value = dither_table_column[i];
        result |= ((transfer_value + dither_value) & 0x100) >> (7 - i);
    }

    return result |
           ((transfer_table[source_row_pointer[7]] + dither_table_column[7]) >>
            8);
}

void set_default_screen()
{
    set_dither_gray_table((std::int8_t*)&device_best_dither, 16, 16);
}

void halftone_dib_to_dib(std::uint8_t*       source_planes8,
                         std::uint8_t*       destination_planes,
                         const std::size_t&  image_width,
                         const std::size_t&  num_rows,
                         const std::int32_t& contrast,
                         const std::int32_t& brightness)
{
    if (dither_table.empty())
    {
        set_default_screen();
    }

    std::array<std::uint8_t, dither_table_size> transfer_table{};

    for (std::size_t i = 0; i < transfer_table.size(); i++)
    {
        transfer_table[i] = apply_transfer_function(
            static_cast<std::uint8_t>(i), contrast, brightness);
    }

    const std::size_t bytes_per_row = (image_width + 7) / 8;

    const std::uint32_t shifted_width =
        static_cast<std::uint32_t>(image_width) >> 31;
    const std::uint32_t adjusted_width =
        static_cast<std::uint32_t>(image_width) + (shifted_width >> 29);

    const std::uint32_t dither_table_row_index =
        (adjusted_width & 7) - (shifted_width >> 29);

    const std::size_t row_index    = get_line_bytes(image_width, 8);
    const std::size_t column_index = get_line_bytes(image_width, 1);

    for (std::size_t i = 0; i < num_rows; i++)
    {
        std::uint8_t* dither_table_row =
            &dither_table[(i % dither_table_height) * dither_table_pitch];
        std::uint8_t* source_row_pointer = &source_planes8[i * row_index];
        std::uint8_t* destination_row_pointer =
            &destination_planes[i * column_index];
        std::size_t dither_table_column_index = 0;

        for (std::size_t j = 0; j < bytes_per_row; j++)
        {
            std::uint8_t* dither_table_column =
                dither_table_row + dither_table_column_index;
            *destination_row_pointer = apply_dithering_and_transfer(
                transfer_table.data(), source_row_pointer, dither_table_column);
            dither_table_column_index =
                (dither_table_column_index + 8) % dither_table_width;
            source_row_pointer += 8;
            destination_row_pointer++;
        }

        if (dither_table_row_index > 0)
        {
            std::uint8_t pixel_value = 0;
            for (std::size_t k = 0; k < dither_table_row_index; ++k)
            {
                pixel_value |=
                    ((source_row_pointer[k] +
                      dither_table_row[dither_table_column_index + k]) &
                     0x100) >>
                    (k + 1);
            }
            *destination_row_pointer = pixel_value;
        }
    }
}
