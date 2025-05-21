#include <halfton.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Transfer2
float m_contrast;
float m_brightness;

// SetDitherGrayTable
uint8_t* m_dither_table;

uint32_t m_dither_table_w;
uint32_t m_dither_table_h;
int32_t  m_dither_table_pitch;

uint8_t transfer2(uint8_t value, int32_t contrast, int32_t brightness)
{
    if (contrast)
    {
        const float m_f_contrast =
            contrast * 0.000025f * contrast + 0.0075f * contrast + 1.0f;
        const float c = (value - 128.0f) * m_f_contrast + 128.0f;
        value         = (uint8_t)fminf(fmaxf(roundf(c), 0.0f), 255.0f);
    }
    if (brightness)
    {
        const float m_f_brightness = 0.005f * brightness;
        const float b              = value + m_f_brightness * 255.0f;
        value = (uint8_t)fminf(fmaxf(roundf(b), 0.0f), 255.0f);
    }
    return value;
}

static uint8_t device_best_dither[256] = {
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

void set_dither_gray_table(int8_t* table, uint32_t width, uint32_t height)
{
    if (m_dither_table)
        free(m_dither_table);

    m_dither_table_w = width;
    m_dither_table_h = height;

    // Align to 8-byte boundary if needed
    m_dither_table_pitch = width + (width & 7 ? 8 - (width & 7) : 0);

    size_t size    = m_dither_table_pitch * height;
    m_dither_table = malloc(size);

    if (!m_dither_table)
    {
        // Handle allocation failure
        m_dither_table_w = m_dither_table_h = m_dither_table_pitch = 0;
        return;
    }

    memset(m_dither_table, 0, size);

    for (uint32_t j = 0; j < height; ++j)
    {
        for (uint32_t k = 0; k < m_dither_table_pitch; ++k)
        {
            uint32_t src_idx        = j * width + k % width;
            uint32_t dst_idx        = j * m_dither_table_pitch + k;
            m_dither_table[dst_idx] = (uint8_t)(-1 - table[src_idx]);
        }
    }
}

void set_default_screen(void)
{
    set_dither_gray_table((int8_t*)&device_best_dither, 16, 16);
}

int32_t get_line_bytes(int32_t width, int32_t mult)
{
    return ((mult * width + 31) & ~31) >> 3;
}

void HalftoneDibToDib(uint8_t* restrict planes8,
                      uint8_t* restrict planes,
                      int32_t width,
                      int32_t numver,
                      int32_t contrast,
                      int32_t brightness)
{
    static uint8_t transfer_table[256];
    const size_t   dither_pitch = m_dither_table_pitch;

    if (!m_dither_table)
        set_default_screen();

    // Precompute transfer table once
    for (size_t i = 0; i < 256; ++i)
    {
        transfer_table[i] = transfer2((uint8_t)i, contrast, brightness);
    }

    const div_t   dim         = div(width, 8);
    const int32_t full_blocks = dim.quot;
    const int32_t remainder   = dim.rem;

    for (int32_t j = 0; j < numver; ++j)
    {
        const uint8_t* dither_row =
            &m_dither_table[(j % m_dither_table_h) * dither_pitch];
        const int32_t  line_bytes_8 = get_line_bytes(width, 8);
        const uint8_t* src_row      = &planes8[j * line_bytes_8];
        const int32_t  line_bytes_1 = get_line_bytes(width, 1);
        uint8_t*       dst_row      = &planes[j * line_bytes_1];

        size_t dither_offset = 0;

        // Process full 8-pixel blocks
        for (int32_t k = 0; k < full_blocks; ++k)
        {
            uint8_t        packed = 0;
            const uint8_t* dither = &dither_row[dither_offset];

            packed |= ((transfer_table[src_row[0]] + dither[0]) & 0x100) >> 1;
            packed |= ((transfer_table[src_row[1]] + dither[1]) & 0x100) >> 2;
            packed |= ((transfer_table[src_row[2]] + dither[2]) & 0x100) >> 3;
            packed |= ((transfer_table[src_row[3]] + dither[3]) & 0x100) >> 4;
            packed |= ((transfer_table[src_row[4]] + dither[4]) & 0x100) >> 5;
            packed |= ((transfer_table[src_row[5]] + dither[5]) & 0x100) >> 6;
            packed |= ((transfer_table[src_row[6]] + dither[6]) & 0x100) >> 7;
            packed |= ((transfer_table[src_row[7]] + dither[7]) & 0x100) >> 8;

            *dst_row++    = packed;
            dither_offset = (dither_offset + 8) % m_dither_table_w;
            src_row += 8;
        }

        // Process remainder pixels
        if (remainder)
        {
            uint8_t        packed = 0;
            const uint8_t* dither = &dither_row[dither_offset];

            for (int32_t l = 0; l < remainder; ++l)
            {
                packed |= ((transfer_table[src_row[l]] + dither[l]) & 0x100) >>
                          (l + 1);
            }
            *dst_row = packed;
        }
    }
}
