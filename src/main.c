#include <cups/raster.h>
#include <fcntl.h>

#include "rastertokpsl.h"

/*
 * usage rastertopcl job-id user title copies options [raster_file]
 */
int main(int argc, const char **argv, const char **envp)
{
    int fd;             /* File descriptor */
    cups_raster_t *ras; /* Raster stream for printing */

    // Ensure status messages are not buffered
    setbuf(stderr, NULL);

    // Check command-line arguments
    if (argc < 6 || argc > 7)
    {
        // Incorrect number of arguments; print error message and exit
        _cupsLangPrintFilter(
            stderr, "ERROR",
            _("%s job-id user title copies options [raster_file]"),
            "rastertokpsl");
        return EXIT_FAILURE;
    }

    if (argc == 7)
    {
        fd = open(argv[6], O_RDONLY);
        if (fd == -1)
        {
            _cupsLangPrintFilter(stderr, "ERROR",
                                 _("Unable to open raster file"));
            sleep(1);
            return EXIT_FAILURE;
        }
    }
    else
    {
        fd = STDIN_FILENO;
    }

    ras = cupsRasterOpen(fd, CUPS_RASTER_READ);

    const int pages = rastertokpsl(ras, argv[2], argv[3], atoi(argv[4]), argv[5]);

    cupsRasterClose(ras);
    if (fd != 0)
    {
        close(fd);
    }

    if (pages != 0)
    {
        _cupsLangPrintFilter(stderr, "INFO", _("Ready to print."));
        return EXIT_SUCCESS;
    }
    else
    {
        _cupsLangPrintFilter(stderr, "ERROR", _("No pages found!"));
        return EXIT_FAILURE;
    }
}
