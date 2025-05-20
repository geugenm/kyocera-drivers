#include <ctype.h>
#include <cups/raster.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "rastertokpsl.h"

int main(int argc, const char** argv)
{
    setbuf(stderr, NULL);

    if (argc < 6 || argc > 7)
    {
        _cupsLangPrintFilter(
            stderr,
            "ERROR",
            _("%s job-id user title copies options [raster_file]"),
            "rastertokpsl");
        return EXIT_FAILURE;
    }

    /* Process job name: alnum only, last 20 chars */
    char job_name[21] = { 0 };
    for (const char *src = argv[3], *dst = job_name; *src; ++src)
    {
        if (isalnum((unsigned char)*src))
        {
            size_t len = strlen(job_name);
            if (len >= 20)
                memmove(job_name, job_name + 1, 19);
            strncat(job_name, src, 1);
        }
    }

    /* File handling */
    int fd = (argc == 7) ? open(argv[6], O_RDONLY) : 0;
    if (fd == -1)
    {
        _cupsLangPrintFilter(stderr, "ERROR", _("Unable to open raster file"));
        return EXIT_FAILURE;
    }

    /* Raster processing */
    cups_raster_t* ras = cupsRasterOpen(fd, CUPS_RASTER_READ);
    int pages = rastertokpsl(ras, argv[2], job_name, atoi(argv[4]), argv[5]);

    /* Cleanup */
    cupsRasterClose(ras);
    if (fd > 0)
        close(fd);

    return pages ? EXIT_SUCCESS
                 : (fprintf(stderr, "ERROR: No pages\n"), EXIT_FAILURE);
}
