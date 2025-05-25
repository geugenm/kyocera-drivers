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
    const fs::path pdf_file    = "test.pdf";
    const fs::path raster_file = "test_raster.ras";
    const fs::path output_file = "output.kpsl";

    // Cleanup previous runs
    fs::remove(raster_file);
    fs::remove(output_file);

    // Build commands
    const auto cmd_pdftoraster =
        std::format("/usr/lib/cups/filter/pdftoraster 1 user test_job 1 "
                    "PageSize=A4 {} > {}",
                    pdf_file.string(),
                    raster_file.string());

    const auto cmd_rastertokpsl =
        std::format("./rastertokpsl 1 user test_job 1 PageSize=A4 {} > {}",
                    raster_file.string(),
                    output_file.string());

    // Execute PDF to raster conversion
    std::cout << std::format(
        "{} info: executing pdftoraster: {}\n", src_tag, cmd_pdftoraster);
    const int ret_pdf = std::system(cmd_pdftoraster.c_str());
    if (ret_pdf != 0)
    {
        std::cerr << std::format(
            "{} err: pdftoraster failed ({})\n", src_tag, ret_pdf);
        return EXIT_FAILURE;
    }

    // Execute raster to KPSL conversion
    std::cout << std::format(
        "{} info: executing rastertokpsl: {}\n", src_tag, cmd_rastertokpsl);
    const int ret_kpsl = std::system(cmd_rastertokpsl.c_str());
    if (ret_kpsl != 0)
    {
        std::cerr << std::format(
            "{} err: rastertokpsl failed ({})\n", src_tag, ret_kpsl);
        return EXIT_FAILURE;
    }

    // Verify output
    if (!fs::exists(output_file))
    {
        std::cerr << std::format(
            "{} err: missing output file {}\n", src_tag, output_file.string());
        return EXIT_FAILURE;
    }

    std::cout << std::format(
        "{} info: success, output: {}\n", src_tag, output_file.string());
    return EXIT_SUCCESS;
}
