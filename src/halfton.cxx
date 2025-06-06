#include "halfton.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace halftone
{
struct state
{
    std::vector<uint8_t> dither_table;
    uint32_t             width  = 0;
    uint32_t             height = 0;
    uint32_t             pitch  = 0;

    static constexpr std::array<uint8_t, 256> device_best_dither = {
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

    void init_default()
    {
        set_dither_table(
            reinterpret_cast<const int8_t*>(device_best_dither.data()), 16, 16);
    }

    void set_dither_table(const int8_t* table, uint32_t w, uint32_t h)
    {
        width  = w;
        height = h;
        pitch  = w + (w & 7 ? 8 - (w & 7) : 0);

        dither_table.resize(pitch * h);

        for (uint32_t y = 0; y < h; ++y)
        {
            for (uint32_t x = 0; x < pitch; ++x)
            {
                const uint32_t src_idx = y * w + (x % w);
                dither_table[y * pitch + x] =
                    static_cast<uint8_t>(0xFF - table[src_idx]);
            }
        }
    }
};

[[nodiscard]] static constexpr uint8_t transfer_pixel(
    uint8_t val, int32_t contrast, int32_t brightness) noexcept
{
    float result = static_cast<float>(val);

    if (contrast != 0)
    {
        const float factor = contrast * (0.000025f * contrast + 0.0075f) + 1.0f;
        result             = (result - 128.0f) * factor + 128.0f;
    }

    if (brightness != 0)
    {
        result += 1.275f * brightness;
    }

    return static_cast<uint8_t>(std::clamp(std::round(result), 0.0f, 255.0f));
}

[[nodiscard]] static constexpr int32_t calculate_line_bytes(
    int32_t width, int32_t mult) noexcept
{
    return ((mult * width + 31) & ~31) >> 3;
}
} // namespace halftone

extern "C" void halftone_dib_to_dib(uint8_t* __restrict planes8,
                                    uint8_t* __restrict planes,
                                    int32_t width,
                                    int32_t numver,
                                    int32_t contrast,
                                    int32_t brightness)
{
    static halftone::state state;
    if (state.dither_table.empty())
    {
        state.init_default();
    }

    std::array<uint8_t, 256> transfer_table;
    for (size_t i = 0; i < 256; ++i)
    {
        transfer_table[i] = halftone::transfer_pixel(
            static_cast<uint8_t>(i), contrast, brightness);
    }

    const int32_t full_blocks = width / 8;
    const int32_t remainder   = width % 8;

    for (int32_t row = 0; row < numver; ++row)
    {
        const uint8_t* dither_row =
            &state.dither_table[(row % state.height) * state.pitch];
        const uint8_t* src_row =
            &planes8[row * halftone::calculate_line_bytes(width, 8)];
        uint8_t* dst_row =
            &planes[row * halftone::calculate_line_bytes(width, 1)];
        size_t dither_pos = 0;

        for (int32_t block = 0; block < full_blocks; ++block)
        {
            uint8_t packed = 0;
            for (int i = 0; i < 8; ++i)
            {
                packed |=
                    ((transfer_table[src_row[i]] + dither_row[dither_pos + i]) &
                     0x100) >>
                    (i + 1);
            }
            *dst_row++ = packed;
            dither_pos = (dither_pos + 8) % state.width;
            src_row += 8;
        }

        if (remainder > 0)
        {
            uint8_t packed = 0;
            for (int i = 0; i < remainder; ++i)
            {
                packed |=
                    ((transfer_table[src_row[i]] + dither_row[dither_pos + i]) &
                     0x100) >>
                    (i + 1);
            }
            *dst_row = packed;
        }
    }
}