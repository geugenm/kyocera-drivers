#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <source_location>
#include <string>

int main()
{
    namespace fs       = std::filesystem;
    const auto src_tag = std::format(
        "[{}]",
        fs::path(std::source_location::current().file_name()).stem().string());

    // Configuration
    const fs::path raster_bin  = "./rastertokpsl";
    const fs::path pdf_file    = "test.pdf";
    const fs::path ppd_file    = "Kyocera_FS-1020MFPGDI.ppd";
    const fs::path output_file = "debug_output.kpsl";

    // Clean previous run
    fs::remove(output_file);

    // Build command pipeline
    const std::string full_cmd =
        std::format("cupsfilter -p {} -m application/vnd.cups-raster {} | {} "
                    "999 debug_user \"Debug_Job\" 1 \"debug_mode=1\" > {}",
                    ppd_file.string(),
                    pdf_file.string(),
                    raster_bin.string(),
                    output_file.string());

    std::cout << std::format(
        "{} info: executing pipeline: {}\n", src_tag, full_cmd);

    // Execute and validate
    const int ret = std::system(full_cmd.c_str());
    if (ret != 0)
    {
        std::cerr << std::format(
            "{} error: pipeline failed with code {}\n", src_tag, ret);
        return EXIT_FAILURE;
    }

    if (!fs::exists(output_file))
    {
        std::cerr << std::format("{} error: missing output file {}\n",
                                 src_tag,
                                 output_file.string());
        return EXIT_FAILURE;
    }

    std::cout << std::format("{} info: success, output written to {}\n"
                             "{} info: inspect with:\n"
                             "  less {}\n"
                             "  hexdump -C {}\n",
                             src_tag,
                             output_file.string(),
                             src_tag,
                             output_file.string(),
                             output_file.string());

    return EXIT_SUCCESS;
}
