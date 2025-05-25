#include "rastertokpsl.h"

#include <halfton.h>
#include <libjbig/jbig.h>
#include <unicode/ConvertUTF.h>

#include <cups/cups.h>
#include <cups/raster.h>

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>

#if !defined(LOBYTE)
#define LOBYTE(w) ((unsigned char)(w))
#endif
#if !defined(HIBYTE)
#define HIBYTE(w) ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))
#endif

#if !defined(LOWORD)
#define LOWORD(d) ((unsigned short)(d))
#endif
#if !defined(HIWORD)
#define HIWORD(d) ((unsigned short)((((unsigned long)(d)) >> 16) & 0xFFFF))
#endif

#define LODWORD(q) ((q).u.dwLowDword)
#define HIDWORD(q) ((q).u.dwHighDword)

#define LOBYTE(w) ((unsigned char)(w))
#define HIBYTE(w) ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

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

/*
 * Globals...
 */

int vert_flag;
int end_of_data_flag;
int light[2];

unsigned      processing_page;
int           pages;
int           pdf_flag;
cups_orient_t cups_orientation;
const char*   paper_size_name;

int nup;

unsigned num_ver;
unsigned num_vert_packed;

unsigned char* next_lines;
unsigned       line_size;
unsigned char* lines;

int inside_band_counter;

// StartPage
unsigned width_in_bytes;
unsigned real_plane_size;
unsigned plane_size;
unsigned plane_size_8;

// buffers
unsigned char* planes;
unsigned char* planes_8;
unsigned char* out_buffer;

// SendPlanesData
unsigned y; /* Current line */
unsigned write_j_big_header;
unsigned compressed_length;

void start_page(/*ppd_file_t *ppd,*/ cups_page_header2_t* header)
{
    signed short orientation1, orientation2; // [sp+68h] [bp-18h]@4
    signed short pageSizeEnum;               // [sp+78h] [bp-8h]@1

    pageSizeEnum = 0;
    switch ((int)header->Orientation)
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
    if (paper_size_name)
    {
        if (!strcmp(paper_size_name, "EnvMonarch"))
        {
            pageSizeEnum = 1;
        }
        else if (!strcmp(paper_size_name, "Env10"))
        {
            pageSizeEnum = 2;
        }
        else if (!strcmp(paper_size_name, "EnvDL"))
        {
            pageSizeEnum = 3;
        }
        else if (!strcmp(paper_size_name, "EnvC5"))
        {
            pageSizeEnum = 4;
        }
        else if (!strcmp(paper_size_name, "Executive"))
        {
            pageSizeEnum = 5;
        }
        else if (!strcmp(paper_size_name, "Letter"))
        {
            pageSizeEnum = 6;
        }
        else if (!strcmp(paper_size_name, "Legal"))
        {
            pageSizeEnum = 7;
        }
        else if (!strcmp(paper_size_name, "A4"))
        {
            pageSizeEnum = 8;
        }
        else if (!strcmp(paper_size_name, "B5"))
        {
            pageSizeEnum = 9;
        }
        else if (!strcmp(paper_size_name, "A3"))
        {
            pageSizeEnum = 10;
        }
        else if (!strcmp(paper_size_name, "B4"))
        {
            pageSizeEnum = 11;
        }
        else if (!strcmp(paper_size_name, "Tabloid"))
        {
            pageSizeEnum = 12;
        }
        else if (!strcmp(paper_size_name, "A5"))
        {
            pageSizeEnum = 13;
        }
        else if (!strcmp(paper_size_name, "A6"))
        {
            pageSizeEnum = 14;
        }
        else if (!strcmp(paper_size_name, "B6"))
        {
            pageSizeEnum = 15;
        }
        else if (!strcmp(paper_size_name, "Env9"))
        {
            pageSizeEnum = 16;
        }
        else if (!strcmp(paper_size_name, "EnvPersonal"))
        {
            pageSizeEnum = 17;
        }
        else if (!strcmp(paper_size_name, "ISOB5"))
        {
            pageSizeEnum = 18;
        }
        else if (!strcmp(paper_size_name, "EnvC4"))
        {
            pageSizeEnum = 30;
        }
        else if (!strcmp(paper_size_name, "OficioII"))
        {
            pageSizeEnum = 33;
        }
        else if (!strcmp(paper_size_name, "P16K"))
        {
            pageSizeEnum = 40;
        }
        else if (!strcmp(paper_size_name, "Statement"))
        {
            pageSizeEnum = 50;
        }
        else if (!strcmp(paper_size_name, "Folio"))
        {
            pageSizeEnum = 51;
        }
        else if (!strcmp(paper_size_name, "OficioMX"))
        {
            pageSizeEnum = 42;
        }
        else
        {
            pageSizeEnum = 19;
        }
    }
    width_in_bytes = (unsigned)floor(
        32.0 * ceil((4 * ((header->cupsWidth + 31) >> 5)) / 32.0));
    real_plane_size = width_in_bytes << 8;
    plane_size      = real_plane_size;
    plane_size_8    = plane_size * 8;
    fprintf(stderr, "INFO: StartPage()\n");
    fprintf(stderr,
            "INFO: cupsHeight=%d(0x%X)\n",
            header->cupsHeight,
            header->cupsHeight);
    fprintf(stderr,
            "INFO: cupsWidth=%d(0x%X) 0x%X\n",
            header->cupsWidth,
            header->cupsWidth,
            header->cupsWidth >> 3);
    fprintf(stderr,
            "INFO: WidthInBytes=%d(0x%X)\n",
            width_in_bytes,
            width_in_bytes);
    fprintf(stderr,
            "INFO: iRealPlaneSize=%d(0x%X)\n",
            real_plane_size,
            real_plane_size);
    fprintf(stderr, "INFO: iPlaneSize=%d(0x%X)\n", plane_size, plane_size);
    fprintf(stderr, "INFO: iPlaneSize8=%d(0x%X)\n", plane_size_8, plane_size_8);

    planes = (unsigned char*)malloc(plane_size);
    memset(planes, 0, plane_size);
    planes_8 = (unsigned char*)malloc(plane_size_8);
    memset(planes_8, 0, plane_size_8);
    lines = (unsigned char*)malloc(8 * width_in_bytes);
    memset(lines, 0, 8 * width_in_bytes);
    next_lines = (unsigned char*)malloc(8 * width_in_bytes);
    memset(next_lines, 0, 8 * width_in_bytes);
    out_buffer = (unsigned char*)malloc(0x100000);
    memset(out_buffer, 0, 0x100000);
    printf("\x1B$0P");   // fwrite("\x1B$0P", 1, 4, fp);
    pwrite_int_start(3); // fprintf(fp, "%c%c%c%c@@@@", 3, 0, 0, 0);
    pwrite_short(
        orientation1); // fprintf(fp, "%c%c", LOBYTE(v15), HIBYTE(v15));
    pwrite_short(
        orientation2); // fprintf(fp, "%c%c", LOBYTE(v16), HIBYTE(v16));
    unsigned short metricWidth =
        (unsigned short)floor(10.0 * (header->PageSize[0] * 0.352777778));
    unsigned short metricHeight =
        (unsigned short)floor(10.0 * (header->PageSize[1] * 0.352777778));
    fprintf(stderr, "INFO: metricWidth=%d\n", metricWidth);
    fprintf(stderr, "INFO: metricHeight=%d\n", metricHeight);
    pwrite_short(metricWidth); // fprintf(fp, "%c%c", LOBYTE(v11), HIBYTE(v11));
    pwrite_short(metricHeight); // fprintf(fp, "%c%c", LOBYTE(v12),
                                // HIBYTE(v12));
    pwrite_short(pageSizeEnum); // fprintf(fp, "%c%c", LOBYTE(v17),
                                // HIBYTE(v17));
    pwrite_short(
        header->cupsMediaType); // fprintf(fp, "%c%c", LOBYTE(h->cupsMediaType),
                                // HIBYTE(h->cupsMediaType));
}

void end_page(int sectionEnd)
{
    fprintf(stderr, "INFO: EndPage()\n");
    printf("\x1B$0F");   // fwrite("\x1B$0F", 1, 4, fp);
    pwrite_int_start(1); // fprintf(fp, "%c%c%c%c@@@@", 1, 0, 0, 0);

    fprintf(stderr, "INFO: sectionEndFlag=%d\n", sectionEnd);
    pwrite_int(sectionEnd); // fprintf(fp, "%c%c%c%c", LOBYTE(sectionEndFlag),
                            // HIBYTE(sectionEndFlag), LOBYTE(sectionEndFlag >>
                            // 16), HIBYTE( sectionEndFlag >> 16));
    fflush(stdout);
    free(planes);
    free(planes_8);
    free(lines);
    free(next_lines);
    if (out_buffer != 0)
    {
        free(out_buffer);
    }
}

void shutdown_process(void)
{
    printf("%c%c", '\x1B', 'E');
}

void cancel_job(int sig)
{
    for (int i = 0; i <= 599; ++i)
        putchar(0);
    end_page(1);
    shutdown_process();
    exit(0);
}

void output_bie(unsigned char* start, size_t len, void* file)
{
    if (write_j_big_header && len == 20)
    {
        write_j_big_header = 0;
    }
    else
    {
        size_t         v3  = len;
        unsigned char* out = out_buffer + compressed_length;
        unsigned char* in  = start;
        while (v3)
        {
            *out++ = *in++;
            --v3;
        }
        compressed_length += len;
    }
}

void send_planes_data(cups_page_header2_t* header)
{
    int          v26; // [sp+ECh] [bp-24h]@13
    unsigned int v27; // [sp+F0h] [bp-20h]@27

    if (header->cupsCompression)
    {
        memcpy(planes_8 + 8 * width_in_bytes * inside_band_counter,
               lines,
               8 * width_in_bytes);

        if ((y && inside_band_counter == 255) || (header->cupsHeight - 1 == y))
        {
            if (y && inside_band_counter == 255)
            {
                halftone_dib_to_dib(planes_8,
                                    planes,
                                    8 * width_in_bytes,
                                    256,
                                    light[1],
                                    light[0]);
            }
            else if (header->cupsHeight - 1 == y)
            {
                halftone_dib_to_dib(planes_8,
                                    planes,
                                    8 * width_in_bytes,
                                    num_ver,
                                    light[1],
                                    light[0]);
            }
            write_j_big_header = 1;
            compressed_length  = 0;
            struct jbg_enc_state encState;
            jbg_enc_init(&encState,
                         8 * width_in_bytes,
                         num_ver,
                         1,
                         &planes,
                         output_bie,
                         stdout);
            jbg_enc_layers(&encState, 0);
            jbg_enc_options(&encState, 0, 0, 256, 0, 0);
            jbg_enc_out(&encState);
            jbg_enc_free(&encState);
            v26 = 32 * (signed int)floor((compressed_length + 31) / 32.0);
            if (plane_size >= v26)
            {
                printf("\x1B$0B");
                pwrite_int_start(
                    v26 / 4 +
                    13); // fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v23),
                         // HIBYTE(v23), LOBYTE(v23 >> 16), HIBYTE(v23 >> 16));
                pwrite_int(1 << 16); // fprintf(fp, "%c%c%c%c", 0, 0, 1, 0);
                pwrite_int(
                    header
                        ->cupsWidth); // fprintf(fp, "%c%c%c%c",
                                      // LOBYTE(printarea_x),
                                      // HIBYTE(printarea_x), LOBYTE(printarea_x
                                      // >> 16), HIBYTE(printarea_x >> 16));
                pwrite_int(
                    width_in_bytes); // fprintf(fp, "%c%c%c%c",
                                     // LOBYTE(WidthInBytes),
                                     // HIBYTE(WidthInBytes),
                                     // LOBYTE(WidthInBytes
                                     // >> 16), HIBYTE(WidthInBytes >> 16));
                pwrite_int(num_ver); // fprintf(fp, "%c%c%c%c", LOBYTE(numVer),
                                     // HIBYTE(numVer), LOBYTE(numVer >> 16),
                                     // HIBYTE(numVer >> 16));
                pwrite_int(num_vert_packed); // fprintf(fp, "%c%c%c%c",
                                             // LOBYTE(numVertPacked),
                                             // HIBYTE(numVertPacked),
                                             // LOBYTE(numVertPacked >> 16),
                                             // HIBYTE(numVertPacked >> 16));
                pwrite_int(1 << 8); // fprintf(fp, "%c%c%c%c", 0, 1, 0, 0);
                pwrite_int(0);      // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                pwrite_int(
                    compressed_length); // fprintf(fp, "%c%c%c%c",
                                        // LOBYTE(compressedLength),
                                        // HIBYTE(compressedLength),
                                        // LOBYTE(compressedLength >> 16),
                                        // HIBYTE(compressedLength >> 16));
                pwrite_int(
                    v26); // fprintf(fp, "%c%c%c%c", LOBYTE(v26), HIBYTE(v26),
                          // LOBYTE(v26 >> 16), HIBYTE(v26 >> 16));
                pwrite_int(0); // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                pwrite_int(
                    y -
                    255); // fprintf(fp, "%c%c%c%c", LOBYTE(v25), HIBYTE(v25),
                          // LOBYTE(v25 >> 16), HIBYTE(v25 >> 16));
                pwrite_int(0); // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                pwrite_int(1); // fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
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
                    num_vert_packed = 256;
                    real_plane_size = num_ver * width_in_bytes;
                    plane_size      = num_ver * width_in_bytes;
                    plane_size_8    = 8 * num_ver * width_in_bytes;
                }
            }
            else
            {
                printf("\x1B$0R");
                pwrite_int_start(
                    plane_size / 4 +
                    10); // fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v23),
                         // HIBYTE(v23), LOBYTE(v23 >> 16), HIBYTE(v23 >> 16));
                pwrite_int(
                    header
                        ->cupsWidth); // fprintf(fp, "%c%c%c%c",
                                      // LOBYTE(printarea_x),
                                      // HIBYTE(printarea_x), LOBYTE(printarea_x
                                      // >> 16), HIBYTE(printarea_x >> 16));
                pwrite_int(
                    width_in_bytes); // fprintf(fp, "%c%c%c%c",
                                     // LOBYTE(WidthInBytes),
                                     // HIBYTE(WidthInBytes),
                                     // LOBYTE(WidthInBytes
                                     // >> 16), HIBYTE(WidthInBytes >> 16));
                pwrite_int(num_ver); // fprintf(fp, "%c%c%c%c", LOBYTE(numVer),
                                     // HIBYTE(numVer), LOBYTE(numVer >> 16),
                                     // HIBYTE(numVer >> 16));
                pwrite_int(num_vert_packed); // fprintf(fp, "%c%c%c%c",
                                             // LOBYTE(numVertPacked),
                                             // HIBYTE(numVertPacked),
                                             // LOBYTE(numVertPacked >> 16),
                                             // HIBYTE(numVertPacked >> 16));
                pwrite_int(real_plane_size); // fprintf(fp, "%c%c%c%c",
                                             // LOBYTE(iRealPlaneSize),
                                             // HIBYTE(iRealPlaneSize),
                                             // LOBYTE(iRealPlaneSize >> 16),
                                             // HIBYTE(iRealPlaneSize >> 16));
                pwrite_int(
                    plane_size); // fprintf(fp, "%c%c%c%c", LOBYTE(iPlaneSize),
                                 // HIBYTE(iPlaneSize), LOBYTE(iPlaneSize >>
                                 // 16), HIBYTE(iPlaneSize >> 16));
                pwrite_int(0);   // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                pwrite_int(
                    y -
                    255); // fprintf(fp, "%c%c%c%c", LOBYTE(v25), HIBYTE(v25),
                          // LOBYTE(v25 >> 16), HIBYTE(v25 >> 16));
                pwrite_int(0); // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
                pwrite_int(1); // fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
                if (y && inside_band_counter == 255)
                {
                    fwrite(planes, 1, width_in_bytes << 8, stdout);
                    memset(planes, 0, width_in_bytes << 8);
                    if (!vert_flag)
                    {
                        num_ver = LOBYTE(header->cupsHeight +
                                         (header->cupsHeight >> 31 >> 24)) -
                                  (header->cupsHeight >> 31 >> 24);
                        num_vert_packed = 256;
                        real_plane_size = num_ver * width_in_bytes;
                        plane_size      = num_ver * width_in_bytes;
                        plane_size_8    = 8 * num_ver * width_in_bytes;
                    }
                }
                else
                {
                    if (header->cupsHeight - 1 == y)
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
        if ((y && inside_band_counter == 255) || (header->cupsHeight - 1 == y))
        {
            printf("\x1B$0R");
            pwrite_int_start(
                plane_size / 4 +
                10); // fprintf(fp, "%c%c%c%c@@@@", LOBYTE(v28), HIBYTE(v28),
                     // LOBYTE(v28 >> 16), HIBYTE(v28 >> 16));
            pwrite_int(
                header->cupsWidth); // fprintf(fp, "%c%c%c%c",
                                    // LOBYTE(printarea_x), HIBYTE(printarea_x),
                                    // LOBYTE(printarea_x >> 16),
                                    // HIBYTE(printarea_x >> 16));
            pwrite_int(
                width_in_bytes); // fprintf(fp, "%c%c%c%c",
                                 // LOBYTE(WidthInBytes), HIBYTE(WidthInBytes),
                                 // LOBYTE(WidthInBytes >> 16),
                                 // HIBYTE(WidthInBytes >> 16));
            pwrite_int(num_ver); // fprintf(fp, "%c%c%c%c", LOBYTE(numVer),
                                 // HIBYTE(numVer), LOBYTE(numVer >> 16),
                                 // HIBYTE(numVer >> 16));
            pwrite_int(
                num_vert_packed); // fprintf(fp, "%c%c%c%c",
                                  // LOBYTE(numVertPacked),
                                  // HIBYTE(numVertPacked), LOBYTE(numVertPacked
                                  // >> 16), HIBYTE(numVertPacked >> 16));
            pwrite_int(real_plane_size); // fprintf(fp, "%c%c%c%c",
                                         // LOBYTE(iRealPlaneSize),
                                         // HIBYTE(iRealPlaneSize),
                                         // LOBYTE(iRealPlaneSize >> 16),
                                         // HIBYTE(iRealPlaneSize >> 16));
            pwrite_int(
                plane_size); // fprintf(fp, "%c%c%c%c", LOBYTE(iPlaneSize),
                             // HIBYTE(iPlaneSize), LOBYTE(iPlaneSize >> 16),
                             // HIBYTE(iPlaneSize >> 16));
            pwrite_int(0);   // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
            pwrite_int(
                y - 255);  // fprintf(fp, "%c%c%c%c", LOBYTE(v30), HIBYTE(v30),
                           // LOBYTE(v30 >> 16), HIBYTE(v30 >> 16));
            pwrite_int(0); // fprintf(fp, "%c%c%c%c", 0, 0, 0, 0);
            pwrite_int(1); // fprintf(fp, "%c%c%c%c", 1, 0, 0, 0);
        }
        memcpy(planes + (num_ver - inside_band_counter - 1) * width_in_bytes,
               lines,
               width_in_bytes);
        if (y && inside_band_counter == 255)
        {
            fwrite(planes, 1, width_in_bytes << 8, stdout);
            memset(planes, 0, width_in_bytes << 8);
            if (!vert_flag)
            {
                num_ver = LOBYTE(header->cupsHeight +
                                 (header->cupsHeight >> 31 >> 24)) -
                          (header->cupsHeight >> 31 >> 24);
                num_vert_packed = 256;
                real_plane_size = num_ver * width_in_bytes;
                plane_size      = num_ver * width_in_bytes;
                plane_size_8    = 8 * num_ver * width_in_bytes;
            }
        }
        else
        {
            if (header->cupsHeight - 1 == y)
            {
                fwrite(planes, 1, num_ver * width_in_bytes, stdout);
                memset(planes, 0, num_ver * width_in_bytes);
                inside_band_counter = -1;
            }
        }
    }
}

char* timestring(char* out)
{
    char   buffer[14]; // [sp+16h] [bp-22h]@1
    time_t v3 = time(0);

    // struct tm *v4 = localtime(&v3);
    strftime((char*)&buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&v3));
    return strncpy(out, (char*)&buffer, sizeof(buffer));
}

/// \brief Main rastertokpsl print job handler.
/// \details
///  - No PPD required (unlike classic CUPS filters).
///  - Handles job setup, option parsing, and page processing.
///  - Reports all operational info to stderr for CUPS logging.
///  - Designed for CUPS raster to Kyocera KPSL conversion.
/// \param ras Raster stream for printing.
/// \param user Username string.
/// \param title Job title string.
/// \param copies Number of copies.
/// \param opts CUPS options string.
/// \return Number of pages processed.
uint32_t rastertokpsl(cups_raster_t* ras,
                      const char*    user,
                      const char*    title,
                      int32_t        copies,
                      const char*    opts)
{
    cups_page_header2_t header;
    struct sigaction    sa = { 0 };
    sa.sa_handler          = cancel_job;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    // Unbuffered status messages for real-time CUPS log feedback.
    setbuf(stderr, NULL);

    int            num_options = 0;
    cups_option_t* options     = NULL;
    num_options                = cupsParseOptions(opts, 0, &options);

    processing_page = 0;
    while (cupsRasterReadHeader2(ras, &header))
    {
        const char* value = NULL;
        ++processing_page;
        vert_flag = 1;

        if (processing_page == 1)
        {
            // --- Job setup: header, user, time, options, etc. ---
            printf(
                "%c%c%c%c%c%c%c%c", 'L', 'S', 'P', 'K', '\x1B', '$', '0', 'J');
            pwrite_int_start_doc('\r');

            UTF16       buffer[64] = { 0 };
            UTF16*      pbuffer    = buffer;
            const UTF8* parg       = (const UTF8*)user;
            ConvertUTF8toUTF16(&parg,
                               parg + strlen(user),
                               &pbuffer,
                               pbuffer + 16,
                               strictConversion);
            fwrite(buffer, 2, 16, stdout);

            char buf_time[14];
            timestring(buf_time);
            fwrite(buf_time, 1, sizeof(buf_time), stdout);
            pwrite_short(0); // 2 nulls

            // Brightness/contrast from options
            value    = cupsGetOption("CaBrightness", num_options, options);
            light[0] = value ? -atoi(value) : 0;
            fprintf(stderr, "info: CaBrightness=%d\n", light[0]);

            value    = cupsGetOption("CaContrast", num_options, options);
            light[1] = value ? atoi(value) : 0;
            fprintf(stderr, "info: CaContrast=%d\n", light[1]);

            value = cupsGetOption(
                "com.apple.print.PrintSettings.PMTotalBeginPages..n.",
                num_options,
                options);
            pages    = value ? atoi(value) : 0;
            pdf_flag = value ? 0 : 1;
            fprintf(stderr, "info: pages=%d\n", pages);
            fprintf(stderr, "info: pdfFlag=%d\n", pdf_flag);

            // N-Up logic (default 1)
            int nup_col = 0, nup_row = 0;
            value = cupsGetOption(
                "com.apple.print.PrintSettings.PMLayoutColumns..n.",
                num_options,
                options);
            if (value)
                nup_col = atoi(value);
            value =
                cupsGetOption("com.apple.print.PrintSettings.PMLayoutRows..n.",
                              num_options,
                              options);
            if (value)
                nup_row = atoi(value);
            nup = nup_row * nup_col;
            if (nup == 0)
                nup = 1;
            fprintf(stderr, "info: PMLayoutColumns=%d\n", nup_col);
            fprintf(stderr, "info: PMLayoutRows=%d\n", nup_row);
            fprintf(stderr, "info: nup=%d\n", nup);

            printf("\x1B$0D");
            pwrite_int_start(16);
            memset(buffer, 0, sizeof(buffer));
            pbuffer = buffer;
            parg    = (const UTF8*)title;
            ConvertUTF8toUTF16(&parg,
                               parg + strlen(title),
                               &pbuffer,
                               pbuffer + 32,
                               lenientConversion);
            fwrite(buffer, 2, 32, stdout);

            // Copies/collate
            int collate = 0;
            if (strstr(opts, " collate"))
            {
                collate = 1;
                copies  = 1;
            }
            printf("\x1B$0C");
            pwrite_int_start(1);
            pwrite_short(copies);
            pwrite_short(collate);

            // Media, duplex, feeding, engine speed
            printf("\x1B$0S");
            pwrite_int_start(2);
            pwrite_short(header.MediaPosition);
            int duplex = header.Duplex ? header.Tumble + header.Duplex : 0;
            value =
                cupsGetOption("com.apple.print.PrintSettings.PMDuplexing..n.",
                              num_options,
                              options);
            if (value)
                duplex = atoi(value) - 1;
            pwrite_short(duplex);
            value       = cupsGetOption("Feeding", num_options, options);
            int feeding = value && !strcmp(value, "On");
            pwrite_short(feeding);
            value = cupsGetOption("EngineSpeed", num_options, options);
            int engine_speed = value && !strcmp(value, "On");
            pwrite_short(engine_speed);
            fprintf(stderr, "info: Duplex=%d\n", duplex);
            fprintf(stderr, "info: Feeding=%d\n", feeding);
            fprintf(stderr, "info: EngineSpeed=%d\n", engine_speed);

            // Resolution
            int w_resolution = 600, h_resolution = 600;
            printf("\x1B$0G");
            pwrite_int_start(3);
            value = cupsGetOption("Resolution", num_options, options);
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

            paper_size_name = cupsGetOption("PageSize", num_options, options);
            if (!paper_size_name)
                paper_size_name = cupsGetOption("media", num_options, options);

            value =
                cupsGetOption("orientation-requested", num_options, options);
            cups_orientation =
                value ? (cups_orient_t)atoi(value) : (cups_orient_t)0;
        }

        if (processing_page > 1)
            end_page(0);

        header.cupsBitsPerColor = 1;
        header.cupsCompression  = 1;
        header.Orientation      = cups_orientation;

        start_page(&header);

        num_ver         = 256;
        num_vert_packed = 256;

        for (y = 0; y < header.cupsHeight; ++y)
        {
            // Progress every 1024 lines
            if ((y & 0x3FF) == 0)
            {
                fprintf(stderr,
                        "info: Printing page %d, %u%% complete.\n",
                        processing_page,
                        100 * y / header.cupsHeight);
                fprintf(stderr,
                        "attr: job-media-progress=%u\n",
                        100 * y / header.cupsHeight);
            }

            if (cupsRasterReadPixels(ras, next_lines, header.cupsBytesPerLine) <
                1)
                break;

            inside_band_counter = LOBYTE(y + (y >> 31 >> 24)) - (y >> 31 >> 24);
            if (vert_flag && header.cupsHeight - y <= 0xFF)
                vert_flag = 0;

            memcpy(lines, next_lines, header.cupsBytesPerLine);
            send_planes_data(&header);
        }
    }

    end_page(1);
    cupsFreeOptions(num_options, options);

    printf("\x1B$0E");
    pwrite_int_start(0);

    printf("\x1B$0T");
    pwrite_int_start(0);

    return processing_page;
}