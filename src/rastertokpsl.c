#include <cups/cups.h>
#include <cups/raster.h>

#include <fcntl.h>
#include <math.h>
#include <signal.h>

#include <ConvertUTF.h>
#include <jbig.h>

#include "halfton.h"
#include "rastertokpsl.h"

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

int vertFlag;
int endOfDataFlag;
int light[2];

unsigned Page;
int pages;
int pdfFlag;
cups_orient_t Orientation;
const char *paperSizeName;

int nup;

unsigned numVer;
unsigned numVertPacked;

unsigned char *nextLines;
unsigned iLineSize;
unsigned char *Lines;

// int skipFlag;
int insideBandCounter;

// StartPage
unsigned WidthInBytes;
unsigned iRealPlaneSize;
unsigned iPlaneSize;
unsigned iPlaneSize8;

// buffers
unsigned char *Planes;
unsigned char *Planes8;
unsigned char *OutBuffer;

// SendPlanesData
unsigned y; /* Current line */
unsigned fWriteJBigHeader;
unsigned compressedLength;

void start_page(cups_page_header2_t *page_header)
{
    signed short orientation1, orientation2;
    signed short pageSizeEnum;

    pageSizeEnum = 0;
    switch ((int)page_header->Orientation)
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
    if (paperSizeName)
    {
        if (!strcmp(paperSizeName, "EnvMonarch"))
        {
            pageSizeEnum = 1;
        }
        else if (!strcmp(paperSizeName, "Env10"))
        {
            pageSizeEnum = 2;
        }
        else if (!strcmp(paperSizeName, "EnvDL"))
        {
            pageSizeEnum = 3;
        }
        else if (!strcmp(paperSizeName, "EnvC5"))
        {
            pageSizeEnum = 4;
        }
        else if (!strcmp(paperSizeName, "Executive"))
        {
            pageSizeEnum = 5;
        }
        else if (!strcmp(paperSizeName, "Letter"))
        {
            pageSizeEnum = 6;
        }
        else if (!strcmp(paperSizeName, "Legal"))
        {
            pageSizeEnum = 7;
        }
        else if (!strcmp(paperSizeName, "A4"))
        {
            pageSizeEnum = 8;
        }
        else if (!strcmp(paperSizeName, "B5"))
        {
            pageSizeEnum = 9;
        }
        else if (!strcmp(paperSizeName, "A3"))
        {
            pageSizeEnum = 10;
        }
        else if (!strcmp(paperSizeName, "B4"))
        {
            pageSizeEnum = 11;
        }
        else if (!strcmp(paperSizeName, "Tabloid"))
        {
            pageSizeEnum = 12;
        }
        else if (!strcmp(paperSizeName, "A5"))
        {
            pageSizeEnum = 13;
        }
        else if (!strcmp(paperSizeName, "A6"))
        {
            pageSizeEnum = 14;
        }
        else if (!strcmp(paperSizeName, "B6"))
        {
            pageSizeEnum = 15;
        }
        else if (!strcmp(paperSizeName, "Env9"))
        {
            pageSizeEnum = 16;
        }
        else if (!strcmp(paperSizeName, "EnvPersonal"))
        {
            pageSizeEnum = 17;
        }
        else if (!strcmp(paperSizeName, "ISOB5"))
        {
            pageSizeEnum = 18;
        }
        else if (!strcmp(paperSizeName, "EnvC4"))
        {
            pageSizeEnum = 30;
        }
        else if (!strcmp(paperSizeName, "OficioII"))
        {
            pageSizeEnum = 33;
        }
        else if (!strcmp(paperSizeName, "P16K"))
        {
            pageSizeEnum = 40;
        }
        else if (!strcmp(paperSizeName, "Statement"))
        {
            pageSizeEnum = 50;
        }
        else if (!strcmp(paperSizeName, "Folio"))
        {
            pageSizeEnum = 51;
        }
        else if (!strcmp(paperSizeName, "OficioMX"))
        {
            pageSizeEnum = 42;
        }
        else
        {
            pageSizeEnum = 19;
        }
    }
    WidthInBytes = (unsigned)floor(
        32.0 * ceil((4 * ((page_header->cupsWidth + 31) >> 5)) / 32.0));
    iRealPlaneSize = WidthInBytes << 8;
    iPlaneSize     = iRealPlaneSize;
    iPlaneSize8    = iPlaneSize * 8;
    fprintf(stderr, "INFO: start_page()\n");
    fprintf(stderr, "INFO: cupsHeight=%d(0x%X)\n", page_header->cupsHeight,
            page_header->cupsHeight);
    fprintf(stderr, "INFO: cupsWidth=%d(0x%X) 0x%X\n", page_header->cupsWidth,
            page_header->cupsWidth, page_header->cupsWidth >> 3);
    fprintf(stderr, "INFO: WidthInBytes=%d(0x%X)\n", WidthInBytes,
            WidthInBytes);
    fprintf(stderr, "INFO: iRealPlaneSize=%d(0x%X)\n", iRealPlaneSize,
            iRealPlaneSize);
    fprintf(stderr, "INFO: iPlaneSize=%d(0x%X)\n", iPlaneSize, iPlaneSize);
    fprintf(stderr, "INFO: iPlaneSize8=%d(0x%X)\n", iPlaneSize8, iPlaneSize8);

    Planes = (unsigned char *)malloc(iPlaneSize);
    memset(Planes, 0, iPlaneSize);
    Planes8 = (unsigned char *)malloc(iPlaneSize8);
    memset(Planes8, 0, iPlaneSize8);
    Lines = (unsigned char *)malloc(8 * WidthInBytes);
    memset(Lines, 0, 8 * WidthInBytes);
    nextLines = (unsigned char *)malloc(8 * WidthInBytes);
    memset(nextLines, 0, 8 * WidthInBytes);
    OutBuffer = (unsigned char *)malloc(0x100000);
    memset(OutBuffer, 0, 0x100000);
    printf("\x1B$0P");
    pwrite_int_start(3);
    pwrite_short(orientation1);
    pwrite_short(orientation2);
    unsigned short metricWidth =
        (unsigned short)floor(10.0 * (page_header->PageSize[0] * 0.352777778));
    unsigned short metricHeight =
        (unsigned short)floor(10.0 * (page_header->PageSize[1] * 0.352777778));
    fprintf(stderr, "INFO: metricWidth=%d\n", metricWidth);
    fprintf(stderr, "INFO: metricHeight=%d\n", metricHeight);
    pwrite_short(metricWidth);
    pwrite_short(metricHeight);
    pwrite_short(pageSizeEnum);
    pwrite_short(page_header->cupsMediaType);
}

void end_page(int section_end)
{
    fprintf(stderr, "INFO: end_page()\n");
    printf("\x1B$0F");
    pwrite_int_start(1);
    fprintf(stderr, "INFO: sectionEndFlag=%d\n", section_end);
    pwrite_int(section_end);
    fflush(stdout);
    free(Planes);
    free(Planes8);
    free(Lines);
    free(nextLines);
    if (OutBuffer != 0)
    {
        free(OutBuffer);
    }
}

void shutdown_printer()
{
    printf("%c%c", '\x1B', 'E');
}

void cancel_job(int signal)
{
    for (int i = 0; i <= 599; ++i)
    {
        putchar(0);
    }
    end_page(1);
    shutdown_printer();
    exit(0);
}

void OutputBie(unsigned char *start, size_t len, void *file)
{
    if (fWriteJBigHeader && len == 20)
    {
        fWriteJBigHeader = 0;
    }
    else
    {
        size_t v3          = len;
        unsigned char *out = OutBuffer + compressedLength;
        unsigned char *in  = start;
        while (v3)
        {
            *out++ = *in++;
            --v3;
        }
        compressedLength += len;
    }
}

void SendPlanesData(cups_page_header2_t *header)
{
    int v26;
    unsigned int v27;

    if (header->cupsCompression)
    {
        memcpy(Planes8 + 8 * WidthInBytes * insideBandCounter, Lines,
               8 * WidthInBytes);

        if ((y && insideBandCounter == 255) || (header->cupsHeight - 1 == y))
        {
            if (y && insideBandCounter == 255)
            {
                halftone_dib_to_dib(Planes8, Planes, 8 * WidthInBytes, 256,
                                    light[1], light[0]);
            }
            else if (header->cupsHeight - 1 == y)
            {
                halftone_dib_to_dib(Planes8, Planes, 8 * WidthInBytes, numVer,
                                    light[1], light[0]);
            }
            fWriteJBigHeader = 1;
            compressedLength = 0;
            struct jbg_enc_state encState;
            jbg_enc_init(&encState, 8 * WidthInBytes, numVer, 1, &Planes,
                         OutputBie, stdout);
            jbg_enc_layers(&encState, 0);
            jbg_enc_options(&encState, 0, 0, 256, 0, 0);
            jbg_enc_out(&encState);
            jbg_enc_free(&encState);
            v26 = 32 * (signed int)floor((compressedLength + 31) / 32.0);
            if (iPlaneSize >= v26)
            {
                printf("\x1B$0B");
                pwrite_int_start(v26 / 4 + 13);
                pwrite_int(1 << 16);
                pwrite_int(header->cupsWidth);
                pwrite_int(WidthInBytes);
                pwrite_int(numVer);
                pwrite_int(numVertPacked);
                pwrite_int(1 << 8);
                pwrite_int(0);
                pwrite_int(compressedLength);
                pwrite_int(v26);
                pwrite_int(0);
                pwrite_int(y - 255);
                pwrite_int(0);
                pwrite_int(1);
                if (compressedLength & 0x1F)
                    v27 = 32 - (((LOBYTE(compressedLength) +
                                  ((compressedLength >> 31) >> 27)) &
                                 0x1F) -
                                ((compressedLength >> 31) >> 27));
                else
                    v27 = 0;
                memset(OutBuffer + compressedLength, 0, v27);
                fwrite(OutBuffer, 1, v27 + compressedLength, stdout);
                memset(OutBuffer, 0, 0x100000);
                memset(Planes, 0, numVer * WidthInBytes);
                memset(Planes8, 0, 8 * numVer * WidthInBytes);
                if (!vertFlag)
                {
                    numVer = LOBYTE(header->cupsHeight +
                                    (header->cupsHeight >> 31 >> 24)) -
                             (header->cupsHeight >> 31 >> 24);
                    numVertPacked  = 256;
                    iRealPlaneSize = numVer * WidthInBytes;
                    iPlaneSize     = numVer * WidthInBytes;
                    iPlaneSize8    = 8 * numVer * WidthInBytes;
                }
            }
            else
            {
                printf("\x1B$0R");
                pwrite_int_start(iPlaneSize / 4 + 10);
                pwrite_int(header->cupsWidth);
                pwrite_int(WidthInBytes);
                pwrite_int(numVer);
                pwrite_int(numVertPacked);
                pwrite_int(iRealPlaneSize);
                pwrite_int(iPlaneSize);
                pwrite_int(0);
                pwrite_int(y - 255);
                pwrite_int(0);
                pwrite_int(1);
                if (y && insideBandCounter == 255)
                {
                    fwrite(Planes, 1, WidthInBytes << 8, stdout);
                    memset(Planes, 0, WidthInBytes << 8);
                    if (!vertFlag)
                    {
                        numVer = LOBYTE(header->cupsHeight +
                                        (header->cupsHeight >> 31 >> 24)) -
                                 (header->cupsHeight >> 31 >> 24);
                        numVertPacked  = 256;
                        iRealPlaneSize = numVer * WidthInBytes;
                        iPlaneSize     = numVer * WidthInBytes;
                        iPlaneSize8    = 8 * numVer * WidthInBytes;
                    }
                }
                else
                {
                    if (header->cupsHeight - 1 == y)
                    {
                        fwrite(Planes, 1, numVer * WidthInBytes, stdout);
                        memset(Planes, 0, numVer * WidthInBytes);
                        insideBandCounter = -1;
                    }
                }
            }
        }
    }
    else
    {
        if ((y && insideBandCounter == 255) || (header->cupsHeight - 1 == y))
        {
            printf("\x1B$0R");
            pwrite_int_start(iPlaneSize / 4 + 10);

            pwrite_int(header->cupsWidth);
            pwrite_int(WidthInBytes);
            pwrite_int(numVer);
            pwrite_int(numVertPacked);
            pwrite_int(iRealPlaneSize);
            pwrite_int(iPlaneSize);
            pwrite_int(0);
            pwrite_int(y - 255);
            pwrite_int(0);
            pwrite_int(1);
        }
        memcpy(Planes + (numVer - insideBandCounter - 1) * WidthInBytes, Lines,
               WidthInBytes);
        if (y && insideBandCounter == 255)
        {
            fwrite(Planes, 1, WidthInBytes << 8, stdout);
            memset(Planes, 0, WidthInBytes << 8);
            if (!vertFlag)
            {
                numVer = LOBYTE(header->cupsHeight +
                                (header->cupsHeight >> 31 >> 24)) -
                         (header->cupsHeight >> 31 >> 24);
                numVertPacked  = 256;
                iRealPlaneSize = numVer * WidthInBytes;
                iPlaneSize     = numVer * WidthInBytes;
                iPlaneSize8    = 8 * numVer * WidthInBytes;
            }
        }
        else
        {
            if (header->cupsHeight - 1 == y)
            {
                fwrite(Planes, 1, numVer * WidthInBytes, stdout);
                memset(Planes, 0, numVer * WidthInBytes);
                insideBandCounter = -1;
            }
        }
    }
}

char *time_string(char *out)
{
    char buffer[14];
    time_t v3 = time(0);

    strftime((char *)&buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&v3));
    return strncpy(out, (char *)&buffer, sizeof(buffer));
}

/*
 * usage rastertopcl job-id user title copies options [raster_file]
 * cups_raster_t *ras;                Raster stream for printing
 */
int rastertokpsl(cups_raster_t *raster_stream, const char *user_name,
                 const char *job_title, int copies_number,
                 const char *printing_options)
{
    cups_page_header2_t header; /* Page header from file */

    sigset(SIGTERM, cancel_job);

    setbuf(stderr, 0);

    int num_options        = 0;
    cups_option_t *options = NULL;
    num_options            = cupsParseOptions(printing_options, 0, &options);

    Page = 0;

    while (cupsRasterReadHeader2(raster_stream, &header))
    {
        const char *value = NULL;

        ++Page;

        vertFlag = 1;
        if (Page == 1)
        {

            /*
             * Setup job in the raster read circle - for setup needs data from
             * header!
             */

            printf("%c%c%c%c%c%c%c%c", 'L', 'S', 'P', 'K', '\x1B', '$', '0',
                   'J');
            pwrite_int_start_doc('\r');

            UTF16 buffer[64];
            memset(&buffer, 0, sizeof(buffer));
            UTF16 *pbuffer   = (UTF16 *)&buffer;
            const UTF8 *parg = (UTF8 *)user_name; // argv[2];
            ConversionResult res =
                ConvertUTF8toUTF16(&parg, parg + strlen(user_name), &pbuffer,
                                   pbuffer + sizeof(buffer), strictConversion);
            fwrite(&buffer, 2, 16, stdout);

            char buf_time[14];
            time_string((char *)&buf_time);
            fwrite(&buf_time, 1, sizeof(buf_time), stdout);
            pwrite_short(0);

            value = cupsGetOption("CaBrightness", num_options, options);
            if (value)
                light[0] = -atoi(value);
            else
                light[0] = 0;
            fprintf(stderr, "INFO: CaBrightness=%d\n", light[0]);
            value = cupsGetOption("CaContrast", num_options, options);
            if (value)
                light[1] = atoi(value);
            else
                light[1] = 0;
            fprintf(stderr, "INFO: CaContrast=%d\n", light[1]);

            value = cupsGetOption(
                "com.apple.print.PrintSettings.PMTotalBeginPages..n.",
                num_options, options);
            if (value)
                pages = atoi(value);
            else
                pdfFlag = 1;
            fprintf(stderr, "INFO: pages=%d\n", pages);
            fprintf(stderr, "INFO: pdfFlag=%d\n", pdfFlag);

            /*
             * N-Up printing places multiple document pages on a single printed
             * page CUPS supports 1, 2, 4, 6, 9, and 16-Up formats; the default
             * format is 1-Up lp -o number-up=2 filename
             */

            int nup_col = 0;
            int nup_row = 0;
            value       = cupsGetOption(
                "com.apple.print.PrintSettings.PMLayoutColumns..n.",
                num_options, options);
            if (value)
                nup_col = atoi(value);

            value =
                cupsGetOption("com.apple.print.PrintSettings.PMLayoutRows..n.",
                              num_options, options);
            if (value)
                nup_row = atoi(value);

            nup = nup_row * nup_col;
            if (nup == 0)
                nup = 1;

            fprintf(stderr, "INFO: PMLayoutColumns=%d\n", nup_col);
            fprintf(stderr, "INFO: PMLayoutRows=%d\n", nup_row);
            fprintf(stderr, "INFO: nup=%d\n", nup);

            printf("\x1B$0D");
            pwrite_int_start(16);

            memset(&buffer, 0, sizeof(buffer));
            pbuffer = (UTF16 *)&buffer;
            parg    = (UTF8 *)job_title;
            res =
                ConvertUTF8toUTF16(&parg, parg + strlen(job_title), &pbuffer,
                                   pbuffer + sizeof(buffer), lenientConversion);
            fwrite(&buffer, 2, 0x20, stdout);

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
            printf("\x1B$0C");
            pwrite_int_start(1);
            pwrite_short(copies_number);
            pwrite_short(collate);

            printf("\x1B$0S");
            pwrite_int_start(2);
            pwrite_short(header.MediaPosition);
            int duplex = 0;
            if (header.Duplex)
                duplex = header.Tumble + header.Duplex;
            else
                duplex = 0;
            value =
                cupsGetOption("com.apple.print.PrintSettings.PMDuplexing..n.",
                              num_options, options);
            if (value)
                duplex = atoi(value) - 1;
            pwrite_short(duplex);
            value       = cupsGetOption("Feeding", num_options, options);
            int feeding = value && !strcmp(value, "On");
            pwrite_short(feeding);
            value = cupsGetOption("EngineSpeed", num_options, options);
            int engine_speed = value && !strcmp(value, "On");
            pwrite_short(engine_speed);

            fprintf(stderr, "INFO: Duplex=%d\n", duplex);
            fprintf(stderr, "INFO: Feeding=%d\n", feeding);
            fprintf(stderr, "INFO: EngineSpeed=%d\n", engine_speed);

            int w_resolution = 600;
            int h_resolution = 600;
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

            paperSizeName = cupsGetOption("PageSize", num_options, options);
            if (!paperSizeName)
                paperSizeName = cupsGetOption("media", num_options, options);

            value =
                cupsGetOption("orientation-requested", num_options, options);
            if (value)
                Orientation = (cups_orient_t)atoi(value);
            else
                Orientation = (cups_orient_t)0;
        }
        if (Page > 1)
            // Not last page
            end_page(0);

        header.cupsBitsPerColor = 1;
        header.cupsCompression  = 1;
        header.Orientation      = Orientation;

        /*
         * Start the page...
         */

        start_page(/*ppd,*/ &header);

        // band = header.cupsHeight;
        numVer        = 256;
        numVertPacked = 256;

        /*
         * Loop for each line on the page...
         */

        for (y = 0; y < header.cupsHeight; ++y)
        {
            /*
             * Print progress...
             */

            if ((y & 0x3FF) == 0)
            {
                _cupsLangPrintFilter(stderr, "INFO",
                                     "Printing page %d, %u%% complete.", Page,
                                     100 * y / header.cupsHeight);
                fprintf(stderr, "ATTR: job-media-progress=%u\n",
                        100 * y / header.cupsHeight);
            }

            /*
             * Read a line of graphics...
             */

            if (cupsRasterReadPixels(raster_stream, nextLines,
                                     header.cupsBytesPerLine) < 1)
                break;

            insideBandCounter = LOBYTE(y + (y >> 31 >> 24)) - (y >> 31 >> 24);
            if (vertFlag && header.cupsHeight - y <= 0xFF)
                vertFlag = 0;

            /*
             * Write it to the printer...
             */

            memcpy(Lines, nextLines, header.cupsBytesPerLine);
            SendPlanesData(&header);
        }
    }

    // last page end
    end_page(1);

    printf("\x1B$0E");
    pwrite_int_start(0);

    /*
     * Shutdown the printer...
     */

    printf("\x1B$0T");
    pwrite_int_start(0);

    return Page;
}
