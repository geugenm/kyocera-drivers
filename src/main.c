#include <fcntl.h>
#include <limits.h>

#include <cups/raster.h>

#include "rastertokpsl.h"

/// @usage rastertokpsl job-id user title copies options [raster_file]
int main(int argc, const char** argv, const char** envp)
{
    const char* rastertokpsl_file_path = argv[0];
    if (argc < 6 || argc > 7)
    {
        _cupsLangPrintFilter(
            stderr,
            "ERROR",
            "Usage: %s job-id user title copies options [raster_file]",
            rastertokpsl_file_path);
        return EXIT_FAILURE;
    }

    int file_descriptor = STDIN_FILENO;

    if (argc == 7)
    {
        const char* raster_file_path = argv[6];

        file_descriptor = open(raster_file_path, O_RDONLY);
        if (file_descriptor == -1)
        {
            _cupsLangPrintFilter(stderr,
                                 "ERROR",
                                 "Unable to open raster file: [%s]",
                                 raster_file_path);
            return EXIT_FAILURE;
        }
    }

    cups_raster_t* cups_printing_raster_stream =
        cupsRasterOpen(file_descriptor, CUPS_RASTER_READ);

    if (file_descriptor != STDIN_FILENO)
    {
        close(file_descriptor);
    }

    if (!cups_printing_raster_stream)
    {
        _cupsLangPrintFilter(stderr, "ERROR", "Unable to open cups_raster");
        return EXIT_FAILURE;
    }

    const char* requested_copies_number = argv[4];
    const int   number_base             = 10;
    char*       endptr;

    long copies_number = strtol(requested_copies_number, &endptr, number_base);

    if (requested_copies_number == endptr)
    {
        _cupsLangPrintFilter(
            stderr, "ERROR", "copies_number was not parsed: no digits found");
        return EXIT_FAILURE;
    }

    if (copies_number < 0 || copies_number > INT_MAX)
    {
        _cupsLangPrintFilter(
            stderr, "ERROR", "Number of copies=%d out of range", copies_number);
        return EXIT_FAILURE;
    }

    const char* user_name        = argv[2];
    const char* job_title        = argv[3];
    const char* printing_options = argv[5];

    const int pages = rastertokpsl(cups_printing_raster_stream,
                                   user_name,
                                   job_title,
                                   (int)copies_number,
                                   printing_options);

    cupsRasterClose(cups_printing_raster_stream);

    if (pages == 0)
    {
        _cupsLangPrintFilter(stderr, "ERROR", "No pages found!");
        return EXIT_FAILURE;
    }

    _cupsLangPrintFilter(stderr, "INFO", "Ready to print.");
    return EXIT_SUCCESS;
}
