#include "rastertokpsl.h"

#include <algorithm>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <unistd.h>

#include <cups/raster.h>

int main(int argc, const char* argv[])
{
    const auto source_tag = std::format(
        "[{}]",
        std::filesystem::path(std::source_location::current().file_name())
            .stem()
            .string());

    const auto executable_path = std::filesystem::absolute(argv[0]);
    std::cerr << std::format(
        "{} inf: executable: {:?}\n", source_tag, executable_path.string());

    if (argc < 6 || argc > 7)
    {
        std::cerr << std::format(
            "{} error: invalid argument count ({}), usage: {} job_id user "
            "title copies options [raster_file]\n",
            source_tag,
            argc,
            executable_path.filename().string());
        return EXIT_FAILURE;
    }

    const std::string_view job_id         = argv[1];
    const std::string_view user_name      = argv[2];
    const std::string_view original_title = argv[3];
    const int32_t          copy_count     = std::atoi(argv[4]);
    const std::string_view print_options  = argv[5];

    std::cerr << std::format("{} dbg: args: job_id={} user={} copies={}\n",
                             source_tag,
                             job_id,
                             user_name,
                             copy_count);

    std::string processed_job_name;
    std::ranges::copy_if(std::string_view(original_title),
                         std::back_inserter(processed_job_name),
                         [](unsigned char c) { return std::isalnum(c); });

    if (processed_job_name.size() > 20)
    {
        processed_job_name =
            processed_job_name.substr(processed_job_name.size() - 20);
    }

    std::cerr << std::format("{} inf: name: {:?} -> {:?}\n",
                             source_tag,
                             original_title,
                             processed_job_name);

    int32_t raster_fd = 0;
    if (argc == 7)
    {
        const auto raster_file_path = std::filesystem::absolute(argv[6]);
        std::cerr << std::format(
            "{} inf: opening {:?}\n", source_tag, raster_file_path.string());

        raster_fd = open(raster_file_path.c_str(), O_RDONLY);
        if (raster_fd == -1)
        {
            std::cerr << std::format("{} error: open failed: {}\n",
                                     source_tag,
                                     std::strerror(errno));
            return EXIT_FAILURE;
        }
    }

    cups_raster_t* raster_stream = cupsRasterOpen(raster_fd, CUPS_RASTER_READ);
    const int32_t  pages_processed = rastertokpsl(raster_stream,
                                                 user_name.data(),
                                                 processed_job_name.c_str(),
                                                 copy_count,
                                                 print_options.data());
    cupsRasterClose(raster_stream);

    if (raster_fd > 0)
    {
        close(raster_fd);
    }

    std::cerr << std::format(
        "{} inf: processed {} pages\n", source_tag, pages_processed);
    return pages_processed > 0
               ? EXIT_SUCCESS
               : (std::cerr << std::format("{} error: 0 pages processed\n",
                                           source_tag),
                  EXIT_FAILURE);
}