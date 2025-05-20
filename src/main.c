// Kyocera KPSL filter for CUPS
// Copyright 2025 by [Your Name]
// SPDX-License-Identifier: Apache-2.0

#define _POSIX_C_SOURCE 200809L
#include "rastertokpsl.h"
#include <cups/raster.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int run_filter(int argc, char* argv[])
{
    setvbuf(stderr, NULL, _IONBF, 0); // Unbuffered error output

    if (argc < 6 || argc > 7)
    {
        _cupsLangPrintFilter(
            stderr,
            "ERROR",
            _("%s job-id user title copies options [raster_file]"),
            "rastertokpsl");
        return EXIT_FAILURE;
    }

    const int fd =
        (argc == 7) ? open(argv[6], O_RDONLY | O_CLOEXEC) : STDIN_FILENO;

    if (fd == -1)
    {
        _cupsLangPrintFilter(
            stderr, "ERROR", _("Failed to open raster file: %s"), argv[6]);
        return EXIT_FAILURE;
    }

    cups_raster_t* const ras = cupsRasterOpen(fd, CUPS_RASTER_READ);
    if (!ras)
    {
        _cupsLangPrintFilter(
            stderr, "ERROR", _("Failed to initialize raster stream"));
        close(fd);
        return EXIT_FAILURE;
    }

    const int pages =
        rastertokpsl(ras, argv[2], argv[3], strtol(argv[4], NULL, 10), argv[5]);

    cupsRasterClose(ras);

    if (fd != STDIN_FILENO)
    {
        close(fd);
    }

    if (pages > 0)
    {
        _cupsLangPrintFilter(
            stderr, "INFO", _("Processed %d pages for printing"), pages);
        return EXIT_SUCCESS;
    }

    _cupsLangPrintFilter(stderr,
                         "ERROR",
                         (pages == 0) ? _("No pages found")
                                      : _("Processing failed"));
    return EXIT_FAILURE;
}

int main(int argc, char* argv[const static argc], char** envp)
{
    return run_filter(argc, argv);
}
