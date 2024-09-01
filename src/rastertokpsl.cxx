#include <array>
#include <cassert>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <source_location>
#include <unordered_map>

#include <cups/cups.h>
#include <cups/raster.h>

#include "halfton.hxx"
#include "rastertokpsl.hxx"

extern "C"
{
#include <ConvertUTF.h>
#include <jbig.h>
}

// <cups/language-private.h>
extern "C"
{
    extern int _cupsLangPrintFilter(FILE*       file_to_write_to,
                                    const char* non_localized_msg_prefix,
                                    const char* message,
                                    ...) _CUPS_FORMAT(3, 4) _CUPS_PRIVATE;
}
// end <cups/language-private.h>

[[nodiscard]] constexpr std::uint8_t get_low_byte(
    const std::uint16_t w) noexcept
{
    return static_cast<std::uint8_t>(w);
}

[[nodiscard]] constexpr std::uint8_t get_high_byte(
    const std::uint16_t w) noexcept
{
    return static_cast<std::uint8_t>((w >> 8) & 0xFF);
}

void write_short_as_bytes(const std::uint16_t n)
{
    std::printf("%c%c", get_low_byte(n), get_high_byte(n));
}

void pwrite_int_f(const char* format, const std::uint32_t n)
{
    std::printf(format,
                get_low_byte(n),
                get_high_byte(n),
                get_low_byte(n >> 16),
                get_high_byte(n >> 16));
}

void write_int_as_bytes(const std::uint32_t n)
{
    pwrite_int_f("%c%c%c%c", n);
}

void write_int_with_suffix(const std::uint32_t n)
{
    pwrite_int_f("%c%c%c%c@@@@", n);
}

void write_int_with_documentation_suffix(const std::uint32_t n)
{
    pwrite_int_f("%c%c%c%c@@@@0100", n);
}

bool               is_printing_vertical;
std::array<int, 2> light;

uint32_t         current_page    = 0;
int32_t          pages           = 0;
cups_orient_t    Orientation     = {};
std::string_view paper_size_name = {};

uint32_t num_ver         = 0;
uint32_t num_vert_packed = 0;

uint8_t* next_lines = nullptr;
uint8_t* Lines      = nullptr;

int32_t inside_band_counter = 0;

// start page
uint32_t width_in_bytes    = 0;
uint32_t i_real_plane_size = 0;
uint32_t i_plane_size      = 0;
uint32_t i_plane_size_8    = 0;

// buffers
uint8_t* planes     = nullptr;
uint8_t* planes_8   = nullptr;
uint8_t* out_buffer = nullptr;

// send planes data
uint32_t current_line                = 0;
bool     f_should_write_j_big_header = false;
uint32_t compressed_length           = 0;

void print_page_header_info(const cups_page_header2_t* page_header)
{
    std::cerr << "INFO: cupsHeight=" << page_header->cupsHeight << " (0x"
              << std::hex << std::uppercase << page_header->cupsHeight << ")"
              << std::dec << std::endl;

    std::cerr << "INFO: cupsWidth=" << page_header->cupsWidth << " (0x"
              << std::hex << std::uppercase << page_header->cupsWidth << ") "
              << " (0x" << (page_header->cupsWidth >> 3) << ")" << std::dec
              << std::endl;

    std::cerr << "INFO: width_in_bytes=" << width_in_bytes << " (0x" << std::hex
              << std::uppercase << width_in_bytes << ")" << std::dec
              << std::endl;

    std::cerr << "INFO: i_real_plane_size=" << i_real_plane_size << " (0x"
              << std::hex << std::uppercase << i_real_plane_size << ")"
              << std::dec << std::endl;

    std::cerr << "INFO: i_plane_size=" << i_plane_size << " (0x" << std::hex
              << std::uppercase << i_plane_size << ")" << std::dec << std::endl;

    std::cerr << "INFO: i_plane_size_8=" << i_plane_size_8 << " (0x" << std::hex
              << std::uppercase << i_plane_size_8 << ")" << std::dec
              << std::endl;
}

enum class page_size
{
    EnvMonarch = 1,
    Env10,
    EnvDL,
    EnvC5,
    Executive,
    Letter,
    Legal,
    A4,
    B5,
    A3,
    B4,
    Tabloid,
    A5,
    A6,
    B6,
    Env9,
    EnvPersonal,
    ISOB5,
    EnvC4     = 30,
    OficioII  = 33,
    P16K      = 40,
    Statement = 50,
    Folio     = 51,
    OficioMX  = 42,
    Unknown   = 19
};

page_size get_page_size_enum(const std::string_view& size_name)
{
    using enum page_size;
    static const std::unordered_map<std::string_view, page_size>
        paper_size_map = { { "EnvMonarch", EnvMonarch },
                           { "Env10", Env10 },
                           { "EnvDL", EnvDL },
                           { "EnvC5", EnvC5 },
                           { "Executive", Executive },
                           { "Letter", Letter },
                           { "Legal", Legal },
                           { "A4", A4 },
                           { "B5", B5 },
                           { "A3", A3 },
                           { "B4", B4 },
                           { "Tabloid", Tabloid },
                           { "A5", A5 },
                           { "A6", A6 },
                           { "B6", B6 },
                           { "Env9", Env9 },
                           { "EnvPersonal", EnvPersonal },
                           { "ISOB5", ISOB5 },
                           { "EnvC4", EnvC4 },
                           { "OficioII", OficioII },
                           { "P16K", P16K },
                           { "Statement", Statement },
                           { "Folio", Folio },
                           { "OficioMX", OficioMX } };

    auto it = paper_size_map.find(size_name);
    return (it != paper_size_map.end()) ? it->second : Unknown;
}

void start_page(cups_page_header2_t* page_header)
{
    int16_t orientation1{};
    int16_t orientation2{};

    switch (static_cast<size_t>(page_header->Orientation))
    {
        case 5:
            orientation1 = 1;
            orientation2 = 1;
            break;
        case 6:
            orientation1 = 0;
            orientation2 = 2;
            break;
        case 4:
            orientation1 = 1;
            orientation2 = 3;
            break;
        default:
            orientation1 = 0;
            orientation2 = 0;
            break;
    }

    width_in_bytes = static_cast<std::uint32_t>(
        floor(32.0 * ceil((4 * ((page_header->cupsWidth + 31) >> 5)) / 32.0)));
    i_real_plane_size = width_in_bytes << 8;
    i_plane_size      = i_real_plane_size;
    i_plane_size_8    = i_plane_size * 8;

    print_page_header_info(page_header);

    planes = (std::uint8_t*)malloc(i_plane_size);
    memset(planes, 0, i_plane_size);
    planes_8 = (std::uint8_t*)malloc(i_plane_size_8);
    memset(planes_8, 0, i_plane_size_8);
    Lines = (std::uint8_t*)malloc(8 * width_in_bytes);
    memset(Lines, 0, 8 * width_in_bytes);
    next_lines = (std::uint8_t*)malloc(8 * width_in_bytes);
    memset(next_lines, 0, 8 * width_in_bytes);
    out_buffer = (std::uint8_t*)malloc(0x100000);
    memset(out_buffer, 0, 0x100000);

    std::cout << "\x1B$0P" << std::endl;
    write_int_with_suffix(3);
    write_short_as_bytes(orientation1);
    write_short_as_bytes(orientation2);

    const auto get_page_metric =
        [&page_header](const std::size_t metric_index) -> uint16_t
    {
        return static_cast<uint16_t>(std::floor(
            10.0 * (page_header->PageSize[metric_index] * 0.352777778)));
    };

    const auto metric_width  = get_page_metric(0);
    const auto metric_height = get_page_metric(1);

    std::cerr << "INFO: metric width = " << metric_width
              << ", metric height = " << metric_height << std::endl;
    write_short_as_bytes(metric_width);
    write_short_as_bytes(metric_height);

    int16_t selected_page_size = 0;
    if (!paper_size_name.empty())
    {
        selected_page_size = static_cast<int16_t>(
            static_cast<std::int32_t>(get_page_size_enum(paper_size_name)));
    }

    write_short_as_bytes(selected_page_size);
    write_short_as_bytes(page_header->cupsMediaType);
}

void end_page(const std::int32_t section_end)
{
    constexpr std::source_location location = std::source_location::current();
    std::cerr << location.function_name();

    std::cout << "\x1B$0F" << std::endl;

    write_int_with_suffix(1);
    std::cerr << "INFO: section_end_flag=" << section_end << std::endl;
    write_int_as_bytes(section_end);

    fflush(stdout);

    free(planes);
    free(planes_8);
    free(Lines);
    free(next_lines);

    if (out_buffer)
    {
        free(out_buffer);
    }
}

void cancel_job([[maybe_unused]] const std::int32_t signal)
{
    for (size_t i = 0; i < 600; ++i)
    {
        std::cout << '\n';
    }
    end_page(1);

    // sequence to cancel job
    std::cout << '\x1B' << 'E';

    exit(EXIT_SUCCESS);
}

void write_data_to_buffer(std::uint8_t*          start,
                          std::size_t            data_length,
                          [[maybe_unused]] void* place_holder)
{
    if (f_should_write_j_big_header && data_length == 20)
    {
        f_should_write_j_big_header = false;
        return;
    }

    unsigned char*       output_ptr = out_buffer + compressed_length;
    const unsigned char* input_ptr  = start;

    memcpy(output_ptr, input_ptr, data_length);

    compressed_length += data_length;
}

void send_planes_data(cups_page_header2_t* header)
{
    if (header->cupsCompression)
    {
        memcpy(planes_8 + 8 * width_in_bytes * inside_band_counter,
               Lines,
               8 * width_in_bytes);

        if ((current_line && inside_band_counter == 255) ||
            (header->cupsHeight - 1 == current_line))
        {
            std::size_t rows_number = 0;
            if (current_line && inside_band_counter == 255)
            {
                rows_number = 256;
            }
            else if (header->cupsHeight - 1 == current_line)
            {
                rows_number = num_ver;
            }

            halftone_dib_to_dib(planes_8,
                                planes,
                                8 * width_in_bytes,
                                rows_number,
                                light[1],
                                light[0]);

            f_should_write_j_big_header = true;
            compressed_length           = 0;
            struct jbg_enc_state encState
            {
            };

            jbg_enc_init(&encState,
                         8 * width_in_bytes,
                         num_ver,
                         1,
                         &planes,
                         write_data_to_buffer,
                         stdout);
            jbg_enc_layers(&encState, 0);
            jbg_enc_options(&encState, 0, 0, 256, 0, 0);
            jbg_enc_out(&encState);
            jbg_enc_free(&encState);
            const uint32_t v26 = static_cast<uint32_t>(
                32 * floor((compressed_length + 31) / 32.0));
            if (i_plane_size >= v26)
            {
                std::cout << "\x1B$0B" << std::endl;
                write_int_with_suffix(v26 / 4 + 13);
                write_int_as_bytes(1 << 16);
                write_int_as_bytes(header->cupsWidth);
                write_int_as_bytes(width_in_bytes);
                write_int_as_bytes(num_ver);
                write_int_as_bytes(num_vert_packed);
                write_int_as_bytes(1 << 8);
                write_int_as_bytes(0);
                write_int_as_bytes(compressed_length);
                write_int_as_bytes(v26);
                write_int_as_bytes(0);
                write_int_as_bytes(current_line - 255);
                write_int_as_bytes(0);
                write_int_as_bytes(1);

                uint32_t v27{};
                if (compressed_length & 0x1F)
                {
                    v27 = 32 - (((get_low_byte(compressed_length) +
                                  ((compressed_length >> 31) >> 27)) &
                                 0x1F) -
                                ((compressed_length >> 31) >> 27));
                }
                memset(out_buffer + compressed_length, 0, v27);
                fwrite(out_buffer, 1, v27 + compressed_length, stdout);
                memset(out_buffer, 0, 0x100000);
                memset(planes, 0, num_ver * width_in_bytes);
                memset(planes_8, 0, 8 * num_ver * width_in_bytes);
                if (!is_printing_vertical)
                {
                    num_ver = get_low_byte(header->cupsHeight +
                                           (header->cupsHeight >> 31 >> 24)) -
                              (header->cupsHeight >> 31 >> 24);
                    num_vert_packed   = 256;
                    i_real_plane_size = num_ver * width_in_bytes;
                    i_plane_size      = num_ver * width_in_bytes;
                    i_plane_size_8    = 8 * num_ver * width_in_bytes;
                }
            }
            else
            {
                std::cout << "\x1B$0R" << std::endl;
                write_int_with_suffix(i_plane_size / 4 + 10);
                write_int_as_bytes(header->cupsWidth);
                write_int_as_bytes(width_in_bytes);
                write_int_as_bytes(num_ver);
                write_int_as_bytes(num_vert_packed);
                write_int_as_bytes(i_real_plane_size);
                write_int_as_bytes(i_plane_size);
                write_int_as_bytes(0);
                write_int_as_bytes(current_line - 255);
                write_int_as_bytes(0);
                write_int_as_bytes(1);
                if (current_line && inside_band_counter == 255)
                {
                    fwrite(planes, 1, width_in_bytes << 8, stdout);
                    memset(planes, 0, width_in_bytes << 8);
                    if (!is_printing_vertical)
                    {
                        num_ver =
                            get_low_byte(header->cupsHeight +
                                         (header->cupsHeight >> 31 >> 24)) -
                            (header->cupsHeight >> 31 >> 24);
                        num_vert_packed   = 256;
                        i_real_plane_size = num_ver * width_in_bytes;
                        i_plane_size      = num_ver * width_in_bytes;
                        i_plane_size_8    = 8 * num_ver * width_in_bytes;
                    }
                }
                else
                {
                    if (header->cupsHeight - 1 == current_line)
                    {
                        fwrite(planes, 1, num_ver * width_in_bytes, stdout);
                        memset(planes, 0, num_ver * width_in_bytes);
                        inside_band_counter = -1;
                    }
                }
            }
        }
    }
    else
    {
        if ((current_line && inside_band_counter == 255) ||
            (header->cupsHeight - 1 == current_line))
        {
            std::cout << "\x1B$0R" << std::endl;
            write_int_with_suffix(i_plane_size / 4 + 10);

            write_int_as_bytes(header->cupsWidth);
            write_int_as_bytes(width_in_bytes);
            write_int_as_bytes(num_ver);
            write_int_as_bytes(num_vert_packed);
            write_int_as_bytes(i_real_plane_size);
            write_int_as_bytes(i_plane_size);
            write_int_as_bytes(0);
            write_int_as_bytes(current_line - 255);
            write_int_as_bytes(0);
            write_int_as_bytes(1);
        }
        memcpy(planes + (num_ver - inside_band_counter - 1) * width_in_bytes,
               Lines,
               width_in_bytes);
        if (current_line && inside_band_counter == 255)
        {
            fwrite(planes, 1, width_in_bytes << 8, stdout);
            memset(planes, 0, width_in_bytes << 8);
            if (!is_printing_vertical)
            {
                num_ver = get_low_byte(header->cupsHeight +
                                       (header->cupsHeight >> 31 >> 24)) -
                          (header->cupsHeight >> 31 >> 24);
                num_vert_packed   = 256;
                i_real_plane_size = num_ver * width_in_bytes;
                i_plane_size      = num_ver * width_in_bytes;
                i_plane_size_8    = 8 * num_ver * width_in_bytes;
            }
        }
        else
        {
            if (header->cupsHeight - 1 == current_line)
            {
                fwrite(planes, 1, num_ver * width_in_bytes, stdout);
                memset(planes, 0, num_ver * width_in_bytes);
                inside_band_counter = -1;
            }
        }
    }
}

[[nodiscard]] std::string get_time_string()
{
    std::array<char, 15> time_buffer{};
    const std::time_t    now        = std::time(nullptr);
    const std::tm*       local_time = std::localtime(&now);

    std::strftime(
        time_buffer.data(), time_buffer.size(), "%Y%m%d%H%M%S", local_time);

    return { time_buffer.data(), 14 };
}

/// Setup job in the raster read circle - for setup needs data from header!
void setup_first_page(const std::string_view&    user_name,
                      const std::string_view&    job_title,
                      uint32_t                   copies_number,
                      const std::string_view&    printing_options,
                      const cups_page_header2_t& header,
                      cups_option_t*             options,
                      const int                  options_number,
                      const char*                value)
{
    std::cout << "LSPK" << '\x1B' << "$0J" << std::endl;
    write_int_with_documentation_suffix('\r');

    constexpr auto get_utf16_buffer = [](std::string_view input_string)
    {
        std::array<UTF16, 64> output_buffer{};
        auto*                 source_to_start = (UTF16*)&output_buffer;
        const auto*           target_start = (const UTF8*)input_string.data();
        ConversionResult      conversion_result =
            ConvertUTF8toUTF16(&target_start,
                               target_start + input_string.length(),
                               &source_to_start,
                               source_to_start + output_buffer.size(),
                               strictConversion);
        assert(conversion_result == conversionOK);
        return output_buffer;
    };

    const auto utf16_user_name = get_utf16_buffer(user_name);
    fwrite(&utf16_user_name, 2, 16, stdout);

    std::cout << get_time_string();
    write_short_as_bytes(0);

    value = cupsGetOption("CaBrightness", options_number, options);
    if (value)
    {
        light[0] = -std::stoi(value);
    }
    else
    {
        light[0] = 0;
    }

    std::cerr << "INFO: CaBrightness=" << light[0] << '\n';
    value = cupsGetOption("CaContrast", options_number, options);
    if (value)
    {
        light[1] = std::stoi(value);
    }
    else
    {
        light[1] = 0;
    }

    std::cerr << "INFO: CaContrast=" << light[1] << '\n';
    std::cerr << "INFO: pages=" << pages << '\n';

    /*
     * N-Up printing places multiple document pages on a single printed
     * page CUPS supports 1, 2, 4, 6, 9, and 16-Up formats; the default
     * format is 1-Up lp -o number-up=2 filename
     */

    std::cout << "\x1B$0D" << std::endl;

    write_int_with_suffix(16);

    const auto utf16_job_title = get_utf16_buffer(job_title);
    fwrite(&utf16_job_title, 2, 0x20, stdout);

    /*
     * Multiple Copies, normally not collated
     *   lp -n num-copies -o Collate=True filename
     */

    int collate = 0;
    if (printing_options == " collate")
    {
        collate       = 1;
        copies_number = 1;
    }
    std::cout << "\x1B$0C" << std::endl;
    write_int_with_suffix(1);
    write_short_as_bytes(copies_number);
    write_short_as_bytes(collate);

    std::cout << "\x1B$0S" << std::endl;
    write_int_with_suffix(2);
    write_short_as_bytes(header.MediaPosition);
    int duplex = 0;

    if (header.Duplex)
    {
        duplex = header.Tumble + header.Duplex;
    }

    write_short_as_bytes(duplex);
    value             = cupsGetOption("Feeding", options_number, options);
    const int feeding = value && !strcmp(value, "On");
    write_short_as_bytes(feeding);
    value = cupsGetOption("EngineSpeed", options_number, options);
    const int engine_speed = value && !strcmp(value, "On");
    write_short_as_bytes(engine_speed);

    std::cerr << "INFO: duplex=" << duplex << '\n';
    std::cerr << "INFO: feeding=" << feeding << '\n';
    std::cerr << "INFO: engine_speed=" << engine_speed << '\n';

    std::cout << "\x1B$0G" << std::endl;
    write_int_with_suffix(3);
    value = cupsGetOption("Resolution", options_number, options);

    int w_resolution = 600;
    int h_resolution = 600;

    if (value && !strcmp(value, "300dpi"))
    {
        h_resolution = 300;
        w_resolution = 300;
    }
    write_short_as_bytes(w_resolution);
    write_short_as_bytes(h_resolution);
    write_short_as_bytes(1);
    write_short_as_bytes(1);
    write_short_as_bytes(32);
    write_short_as_bytes(1 << 8);

    paper_size_name = cupsGetOption("page_size", options_number, options);
    if (!paper_size_name.empty())
    {
        paper_size_name = cupsGetOption("media", options_number, options);
    }

    value = cupsGetOption("orientation-requested", options_number, options);

    if (!value)
    {
        Orientation = {};
        return;
    }

    Orientation = static_cast<cups_orient_t>(atoi(value));
};

std::size_t rastertokpsl(cups_raster_t*         raster_stream,
                         const std::string_view user_name,
                         const std::string_view job_title,
                         const std::uint32_t    copies_number,
                         const std::string_view printing_options)
{
    signal(SIGTERM, cancel_job);

    cups_option_t*     options = nullptr;
    const std::int32_t options_number =
        cupsParseOptions(printing_options.data(), 0, &options);

    current_page = 0;
    cups_page_header2_t page_header_from_file;
    while (cupsRasterReadHeader2(raster_stream, &page_header_from_file))
    {
        ++current_page;

        is_printing_vertical = true;
        if (current_page == 1)
        {
            setup_first_page(user_name,
                             job_title,
                             copies_number,
                             printing_options,
                             page_header_from_file,
                             options,
                             options_number,
                             nullptr);
        }

        if (current_page > 1)
        {
            // Not last page
            end_page(0);
        }

        page_header_from_file.cupsBitsPerColor = 1;
        page_header_from_file.cupsCompression  = 1;
        page_header_from_file.Orientation      = Orientation;

        start_page(&page_header_from_file);

        num_ver         = 256;
        num_vert_packed = 256;

        for (current_line = 0; current_line < page_header_from_file.cupsHeight;
             ++current_line)
        {
            if ((current_line & 0x3FF) == 0)
            {
                const uint32_t job_media_progress =
                    100 * current_line / page_header_from_file.cupsHeight;
                _cupsLangPrintFilter(stdout,
                                     "INFO",
                                     "Printing page %d, %u%% complete.",
                                     current_page,
                                     job_media_progress);

                std::cerr << "ATTR: job-media-progress=" << job_media_progress;
            }

            if (cupsRasterReadPixels(raster_stream,
                                     next_lines,
                                     page_header_from_file.cupsBytesPerLine) <
                1)
            {
                break;
            }

            inside_band_counter =
                get_low_byte(current_line + (current_line >> 31 >> 24)) -
                (current_line >> 31 >> 24);

            if (is_printing_vertical &&
                page_header_from_file.cupsHeight - current_line <= 0xFF)
            {
                is_printing_vertical = false;
            }

            std::cerr << "writing to the printer...";
            memcpy(Lines, next_lines, page_header_from_file.cupsBytesPerLine);
            send_planes_data(&page_header_from_file);
        }
    }

    std::cerr << "ending last page...";
    end_page(1);

    std::cout << "\x1B$0E" << std::endl;
    write_int_with_suffix(0);

    std::cerr << "shutting down the printer...";
    std::cout << "\x1B$0T" << std::endl;
    write_int_with_suffix(0);

    return current_page;
}
