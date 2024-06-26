#include <cups/raster.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include "rastertokpsl.h"


/// @usage rastertopcl job-id user title copies options [raster_file]
int main(int argc, const char **argv, const char **envp)
{
    if (argc < 6 || argc > 7)
    {
        _cupsLangPrintFilter(
            stderr, "ERROR",
            "Usage: %s job-id user title copies options [raster_file]",
            argv[0]);
        return EXIT_FAILURE;
    }

    int file_descriptor;

    if (argc == 7)
    {
        file_descriptor = open(argv[6], O_RDONLY);
        if (file_descriptor == -1)
        {
            _cupsLangPrintFilter(stderr, "ERROR", "Unable to open raster file");
            return EXIT_FAILURE;
        }
    }
    else
    {
        file_descriptor = STDIN_FILENO;
    }

    cups_raster_t *cups_printing_raster_stream =
        cupsRasterOpen(file_descriptor, CUPS_RASTER_READ);

    if (!cups_printing_raster_stream)
    {
        _cupsLangPrintFilter(stderr, "ERROR", "Unable to open cups_raster");
        return EXIT_FAILURE;
    }

    char *endptr;
    errno = 0;

    long copies_number = strtol(argv[4], &endptr, 10);

    if (errno == ERANGE && (copies_number == LONG_MAX || copies_number == LONG_MIN)) {
        _cupsLangPrintFilter(stderr, "ERROR", "Number of copies out of range");
        return EXIT_FAILURE;
    }

    if (endptr == argv[4] || *endptr != '\0') {
        _cupsLangPrintFilter(stderr, "ERROR", "Invalid number of copies");
        return EXIT_FAILURE;
    }

    if (copies_number > INT_MAX || copies_number < 0) {
        _cupsLangPrintFilter(stderr, "ERROR", "Number of copies out of int32 range");
        return EXIT_FAILURE;
    }

    const int pages = rastertokpsl(cups_printing_raster_stream, argv[2],
                                   argv[3], (int) copies_number, argv[5]);

    cupsRasterClose(cups_printing_raster_stream);
    if (file_descriptor != STDIN_FILENO)
    {
        close(file_descriptor);
    }

    if (pages == 0)
    {
        _cupsLangPrintFilter(stderr, "ERROR", "No pages found!");
        return EXIT_FAILURE;
    }

    _cupsLangPrintFilter(stderr, "INFO", "Ready to print.");
    return EXIT_SUCCESS;
}
