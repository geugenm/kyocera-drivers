#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <iostream>

#include <cups/raster.h>

#include "rastertokpsl.hxx"

int main(int argc, const char** argv)
{
    const std::filesystem::path rastertokpsl_executable = argv[0];
    if (argc < 6 || argc > 7)
    {
        std::cerr << "Usage: " << rastertokpsl_executable
                  << "<job-id> <user> <title> <copies> <options> <raster_file>";
        return EXIT_FAILURE;
    }

    int file_descriptor = STDIN_FILENO;

    if (argc == 7)
    {
        const std::filesystem::path raster_file = argv[6];

        file_descriptor = open(raster_file.c_str(), O_RDONLY);
        if (file_descriptor == -1)
        {
            std::cerr << "Unable to open raster file: [" << raster_file << "]";
            return EXIT_FAILURE;
        }
    }

    std::cerr << "opening cups printing stream...";
    cups_raster_t* cups_printing_raster_stream =
        cupsRasterOpen(file_descriptor, CUPS_RASTER_READ);

    if (file_descriptor != STDIN_FILENO)
    {
        close(file_descriptor);
    }

    if (!cups_printing_raster_stream)
    {
        std::cerr << "Unable to open cups_printing_raster_stream";
        return EXIT_FAILURE;
    }

    const std::string_view requested_copies_number = argv[4];

    uint32_t copies_number{};
    try
    {
        copies_number = std::stoul(requested_copies_number.data());
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: invalid copies number given:" << e.what();
        return EXIT_FAILURE;
    }

    const std::string_view user_name = argv[2];

    std::string job_title = argv[3];
    job_title.erase(std::remove_if(job_title.begin(),
                                   job_title.end(),
                                   [](const char c)
                                   { return !std::isalnum(c); }),
                    job_title.end());

    const std::string_view printing_options = argv[5];

    std::cerr << "starting rastertokpsl with params:"
              << "user_name =" << user_name << ", job_title =" << job_title
              << ", copies_number =" << copies_number
              << ", printing_options =" << printing_options << '\n';

    const std::size_t printed_pages_number =
        rastertokpsl(cups_printing_raster_stream,
                     user_name,
                     job_title,
                     copies_number,
                     printing_options);

    cupsRasterClose(cups_printing_raster_stream);

    if (printed_pages_number == 0)
    {
        std::cerr << "error: printed_pages_number == 0";
        return EXIT_FAILURE;
    }

    std::cout << "ready to print!" << std::endl;
    return std::cout.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
