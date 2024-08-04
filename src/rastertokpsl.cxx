#include <array>
#include <cassert>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <unordered_map>

extern "C"
{
#include <cups/cups.h>
#include <cups/raster.h>

#include <ConvertUTF.h>
#include <jbig.h>

#include "halfton.h"
}

#include "rastertokpsl.hxx"

#define LOBYTE(w) (unsigned char)(w)
#define HIBYTE(w) (unsigned char)(((unsigned short)(w) >> 8) & 0xFF)

#define FORMAT_SHORT "%c%c"
#define FORMAT_INT "%c%c%c%c"
#define FORMAT_INT_START "%c%c%c%c@@@@"
#define FORMAT_INT_START_DOC "%c%c%c%c@@@@0100"
#define pwrite_short(n) printf(FORMAT_SHORT, LOBYTE((n)), HIBYTE((n)))
#define pwrite_int_f(f, n)                                                     \
    printf((f), LOBYTE((n)), HIBYTE((n)), LOBYTE((n >> 16)), HIBYTE((n >> 16)))

#define pwrite_int(n) pwrite_int_f((FORMAT_INT), (n))
#define pwrite_int_start(n) pwrite_int_f((FORMAT_INT_START), (n))
#define pwrite_int_start_doc(n) pwrite_int_f((FORMAT_INT_START_DOC), (n))

int32_t                vert_flag;
std::array<int, 2> light;

uint32_t         current_page;
int32_t              pages;
int32_t              pdf_flag;
cups_orient_t    Orientation;
std::string_view paper_size_name;

int32_t nup;

uint32_t num_ver;
uint32_t num_vert_packed;

uint8_t* next_lines;
uint8_t* Lines;

int32_t inside_band_counter;

// StartPage
uint32_t width_in_bytes;
uint32_t i_real_plane_size;
uint32_t i_plane_size;
uint32_t i_plane_size_8;

// buffers
uint8_t* planes;
uint8_t* planes_8;
uint8_t* out_buffer;

// send_planes_data
uint32_t current_line;
uint32_t f_write_j_big_header;
uint32_t compressed_length;

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
    int16_t orientation1;
    int16_t orientation2;
    int16_t page_size_enum;

    page_size_enum = 0;
    switch (const auto header_orientation =
                static_cast<size_t>(page_header->Orientation))
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

    if (!paper_size_name.empty())
    {
        page_size_enum = static_cast<int16_t>(
            static_cast<int>(get_page_size_enum(paper_size_name)));
    }

    width_in_bytes = (unsigned)floor(
        32.0 * ceil((4 * ((page_header->cupsWidth + 31) >> 5)) / 32.0));
    i_real_plane_size = width_in_bytes << 8;
    i_plane_size      = i_real_plane_size;
    i_plane_size_8    = i_plane_size * 8;

    std::cerr << "INFO: start_page()\n";
    print_page_header_info(page_header);

    planes = (unsigned char*)malloc(i_plane_size);
    memset(planes, 0, i_plane_size);
    planes_8 = (unsigned char*)malloc(i_plane_size_8);
    memset(planes_8, 0, i_plane_size_8);
    Lines = (unsigned char*)malloc(8 * width_in_bytes);
    memset(Lines, 0, 8 * width_in_bytes);
    next_lines = (unsigned char*)malloc(8 * width_in_bytes);
    memset(next_lines, 0, 8 * width_in_bytes);
    out_buffer = (unsigned char*)malloc(0x100000);
    memset(out_buffer, 0, 0x100000);

    std::cout << "\x1B$0P" << std::endl;
    pwrite_int_start(3);
    pwrite_short(orientation1);
    pwrite_short(orientation2);

    const auto get_page_metric =
        [&page_header](const std::size_t metric_index) -> uint16_t
    {
        return static_cast<uint16_t>(
            floor(10.0 * (page_header->PageSize[metric_index] * 0.352777778)));
    };

    const auto metric_width  = get_page_metric(0);
    const auto metric_height = get_page_metric(1);

    std::cerr << "INFO: metric width = " << metric_width
              << ", metric height = " << metric_height << std::endl;
    pwrite_short(metric_width);
    pwrite_short(metric_height);
    pwrite_short(page_size_enum);
    pwrite_short(page_header->cupsMediaType);
}

void end_page(int section_end)
{
    std::cerr << "INFO: end_page()\n";
    std::cout << "\x1B$0F" << std::endl;
    pwrite_int_start(1);
    std::cerr << "INFO: sectionEndFlag=" << section_end << std::endl;
    pwrite_int(section_end);
    fflush(stdout);
    free(planes);
    free(planes_8);
    free(Lines);
    free(next_lines);
    if (out_buffer != nullptr)
    {
        free(out_buffer);
    }
}

void shutdown_printer()
{
    std::cout << '\x1B' << 'E';
}

void cancel_job(int signal)
{
    for (size_t i = 0; i < 600; ++i)
    {
        putchar(0);
    }
    end_page(1);
    shutdown_printer();
    exit(EXIT_SUCCESS);
}

void write_data_to_buffer(unsigned char* start, size_t data_length, void* file)
{
    if (f_write_j_big_header && data_length == 20)
    {
        f_write_j_big_header = 0;
        return;
    }
    size_t         remaining_bytes = data_length;
    unsigned char* output_ptr      = out_buffer + compressed_length;
    unsigned char const* input_ptr       = start;

    while (remaining_bytes)
    {
        *output_ptr++ = *input_ptr++;
        --remaining_bytes;
    }

    compressed_length += data_length;
}

void send_planes_data(cups_page_header2_t* header)
{
    int          v26;
    unsigned int v27;

    if (header->cupsCompression)
    {
        memcpy(planes_8 + 8 * width_in_bytes * inside_band_counter,
               Lines,
               8 * width_in_bytes);

        if ((current_line && inside_band_counter == 255) || (header->cupsHeight - 1 == current_line))
        {
            if (current_line && inside_band_counter == 255)
            {
                halftone_dib_to_dib(planes_8,
                                    planes,
                                    8 * width_in_bytes,
                                    256,
                                    light[1],
                                    light[0]);
            }
            else if (header->cupsHeight - 1 == current_line)
            {
                halftone_dib_to_dib(planes_8,
                                    planes,
                                    8 * width_in_bytes,
                                    num_ver,
                                    light[1],
                                    light[0]);
            }
            f_write_j_big_header = 1;
            compressed_length    = 0;
            struct jbg_enc_state encState{};
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
            v26 = 32 * (signed int)floor((compressed_length + 31) / 32.0);
            if (i_plane_size >= v26)
            {
                std::cout << "\x1B$0B" << std::endl;
                pwrite_int_start(v26 / 4 + 13);
                pwrite_int(1 << 16);
                pwrite_int(header->cupsWidth);
                pwrite_int(width_in_bytes);
                pwrite_int(num_ver);
                pwrite_int(num_vert_packed);
                pwrite_int(1 << 8);
                pwrite_int(0);
                pwrite_int(compressed_length);
                pwrite_int(v26);
                pwrite_int(0);
                pwrite_int(current_line - 255);
                pwrite_int(0);
                pwrite_int(1);
                if (compressed_length & 0x1F)
                    v27 = 32 - (((LOBYTE(compressed_length) +
                                  ((compressed_length >> 31) >> 27)) &
                                 0x1F) -
                                ((compressed_length >> 31) >> 27));
                else
                    v27 = 0;
                memset(out_buffer + compressed_length, 0, v27);
                fwrite(out_buffer, 1, v27 + compressed_length, stdout);
                memset(out_buffer, 0, 0x100000);
                memset(planes, 0, num_ver * width_in_bytes);
                memset(planes_8, 0, 8 * num_ver * width_in_bytes);
                if (!vert_flag)
                {
                    num_ver = LOBYTE(header->cupsHeight +
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
                pwrite_int_start(i_plane_size / 4 + 10);
                pwrite_int(header->cupsWidth);
                pwrite_int(width_in_bytes);
                pwrite_int(num_ver);
                pwrite_int(num_vert_packed);
                pwrite_int(i_real_plane_size);
                pwrite_int(i_plane_size);
                pwrite_int(0);
                pwrite_int(current_line - 255);
                pwrite_int(0);
                pwrite_int(1);
                if (current_line && inside_band_counter == 255)
                {
                    fwrite(planes, 1, width_in_bytes << 8, stdout);
                    memset(planes, 0, width_in_bytes << 8);
                    if (!vert_flag)
                    {
                        num_ver = LOBYTE(header->cupsHeight +
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
        if ((current_line && inside_band_counter == 255) || (header->cupsHeight - 1 == current_line))
        {
            std::cout << "\x1B$0R" << std::endl;
            pwrite_int_start(i_plane_size / 4 + 10);

            pwrite_int(header->cupsWidth);
            pwrite_int(width_in_bytes);
            pwrite_int(num_ver);
            pwrite_int(num_vert_packed);
            pwrite_int(i_real_plane_size);
            pwrite_int(i_plane_size);
            pwrite_int(0);
            pwrite_int(current_line - 255);
            pwrite_int(0);
            pwrite_int(1);
        }
        memcpy(planes + (num_ver - inside_band_counter - 1) * width_in_bytes,
               Lines,
               width_in_bytes);
        if (current_line && inside_band_counter == 255)
        {
            fwrite(planes, 1, width_in_bytes << 8, stdout);
            memset(planes, 0, width_in_bytes << 8);
            if (!vert_flag)
            {
                num_ver = LOBYTE(header->cupsHeight +
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

char* get_time_string(char* output_buffer)
{
    std::array<char, 15> time_buffer  = {};
    time_t               current_time = time(nullptr);
    struct tm*           local_time   = localtime(&current_time);

    strftime(
        time_buffer.data(), time_buffer.size(), "%Y%m%d%H%M%S", local_time);
    return strncpy(output_buffer, time_buffer.data(), time_buffer.size() - 1);
}

/*
 * usage rastertopcl job-id user title copies options [raster_file]
 * cups_raster_t *ras;                Raster stream for printing
 */
std::size_t rastertokpsl(cups_raster_t* raster_stream,
                         const char*    user_name,
                         const char*    job_title,
                         int            copies_number,
                         const char*    printing_options)
{
    cups_page_header2_t header; /* Page header from file */

    signal(SIGTERM, cancel_job);

    cups_option_t* options = nullptr;
    const int num_options  = cupsParseOptions(printing_options, 0, &options);

    current_page = 0;

    while (cupsRasterReadHeader2(raster_stream, &header))
    {
        const char* value = nullptr;

        ++current_page;

        vert_flag = 1;
        if (current_page == 1)
        {
            /*
             * Setup job in the raster read circle - for setup needs data from
             * header!
             */

            std::cout << "LSPK" << '\x1B' << "$0J" << std::endl;
            pwrite_int_start_doc('\r');

            constexpr auto get_utf16_buffer = [](std::string_view input_string)
            {
                std::array<UTF16, 64> output_buffer{};
                auto*                 source_to_start = (UTF16*)&output_buffer;
                const auto* target_start = (const UTF8*)input_string.data();
                ConversionResult conversion_result =
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

            std::string_view buf_time;
            get_time_string((char*)&buf_time);
            fwrite(&buf_time, 1, buf_time.length(), stdout);
            pwrite_short(0);

            value = cupsGetOption("CaBrightness", num_options, options);
            if (value)
            {
                light[0] = -std::atoi(value);
            }
            else
            {
                light[0] = 0;
            }

            std::cout << "INFO: CaBrightness=" << light[0] << '\n';
            value = cupsGetOption("CaContrast", num_options, options);
            if (value)
            {
                light[1] = atoi(value);
            }
            else
            {
                light[1] = 0;
            }

            std::cout << "INFO: CaContrast=" << light[1] << '\n';

            pdf_flag = 1;
            std::cout << "INFO: pages=" << pages << '\n';
            std::cout << "INFO: pdf_flag=" << pdf_flag << '\n';

            /*
             * N-Up printing places multiple document pages on a single printed
             * page CUPS supports 1, 2, 4, 6, 9, and 16-Up formats; the default
             * format is 1-Up lp -o number-up=2 filename
             */

            nup = 1;

            std::cerr << "INFO: nup=" << nup << '\n';

            std::cout << "\x1B$0D" << std::endl;

            pwrite_int_start(16);

            const auto utf16_job_title = get_utf16_buffer(job_title);
            fwrite(&utf16_job_title, 2, 0x20, stdout);

            /*
             * Multiple Copies, normally not collated
             *   lp -n num-copies -o Collate=True filename
             */

            int collate = 0;
            if (strstr(printing_options, " collate"))
            {
                collate       = 1;
                copies_number = 1;
            }
            std::cout << "\x1B$0C" << std::endl;
            pwrite_int_start(1);
            pwrite_short(copies_number);
            pwrite_short(collate);

            std::cout << "\x1B$0S" << std::endl;
            pwrite_int_start(2);
            pwrite_short(header.MediaPosition);
            int duplex = 0;

            if (header.Duplex)
            {
                duplex = header.Tumble + header.Duplex;
            }

            pwrite_short(duplex);
            value             = cupsGetOption("Feeding", num_options, options);
            const int feeding = value && !strcmp(value, "On");
            pwrite_short(feeding);
            value = cupsGetOption("EngineSpeed", num_options, options);
            const int engine_speed = value && !strcmp(value, "On");
            pwrite_short(engine_speed);

            std::cerr << "INFO: duplex=" << duplex << '\n';
            std::cerr << "INFO: feeding=" << feeding << '\n';
            std::cerr << "INFO: engine_speed=" << engine_speed << '\n';

            std::cout << "\x1B$0G" << std::endl;
            pwrite_int_start(3);
            value = cupsGetOption("Resolution", num_options, options);

            int w_resolution = 600;
            int h_resolution = 600;

            if (value && !strcmp(value, "300dpi"))
            {
                h_resolution = 300;
                w_resolution = 300;
            }
            pwrite_short(w_resolution);
            pwrite_short(h_resolution);
            pwrite_short(1);
            pwrite_short(1);
            pwrite_short(32);
            pwrite_short(1 << 8);

            paper_size_name = cupsGetOption("page_size", num_options, options);
            if (!paper_size_name.empty())
                paper_size_name = cupsGetOption("media", num_options, options);

            value =
                cupsGetOption("orientation-requested", num_options, options);
            if (value)
                Orientation = (cups_orient_t)atoi(value);
            else
                Orientation = (cups_orient_t)0;
        }

        if (current_page > 1)
        {
            // Not last page
            end_page(0);
        }

        header.cupsBitsPerColor = 1;
        header.cupsCompression  = 1;
        header.Orientation      = Orientation;

        start_page(&header);

        num_ver         = 256;
        num_vert_packed = 256;

        /*
         * Loop for each line on the page...
         */

        for (current_line = 0; current_line < header.cupsHeight; ++current_line)
        {
            if ((current_line & 0x3FF) == 0)
            {
                const uint32_t job_media_progress = 100 * current_line / header.cupsHeight;
                _cupsLangPrintFilter(stdout,
                                     "INFO",
                                     "Printing page %d, %u%% complete.",
                                     current_page,
                                     job_media_progress);

                std::cout << "ATTR: job-media-progress=" << job_media_progress;
            }

            /*
             * Read a line of graphics...
             */

            if (cupsRasterReadPixels(
                    raster_stream, next_lines, header.cupsBytesPerLine) < 1)
                break;

            inside_band_counter = LOBYTE(current_line + (current_line >> 31 >> 24)) - (current_line >> 31 >> 24);
            if (vert_flag && header.cupsHeight - current_line <= 0xFF)
                vert_flag = 0;

            /*
             * Write it to the printer...
             */

            memcpy(Lines, next_lines, header.cupsBytesPerLine);
            send_planes_data(&header);
        }
    }

    // last page end
    end_page(1);

    std::cout << "\x1B$0E" << std::endl;
    pwrite_int_start(0);

    /*
     * Shutdown the printer...
     */

    std::cout << "\x1B$0T" << std::endl;
    pwrite_int_start(0);

    return current_page;
}
