#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main()
{
    namespace fs = std::filesystem;

    const std::string pdf_file    = "test.pdf";
    const std::string raster_file = "test_raster.ras";
    const std::string output_file = "output.kpsl";

    fs::remove(raster_file);
    fs::remove(output_file);

    std::string cmd_pdftoraster =
        "/usr/lib/cups/filter/pdftoraster 1 user test_job 1 PageSize=A4 " +
        pdf_file + " > " + raster_file;
    std::string cmd_rastertokpsl =
        "./rastertokpsl 1 user test_job 1 PageSize=A4 " + raster_file + " > " +
        output_file;

    std::cout << "running: " << std::quoted(cmd_pdftoraster) << std::endl;
    int ret1 = std::system(cmd_pdftoraster.c_str());
    if (ret1 != 0)
    {
        std::cerr << "error: pdftoraster failed with code " << ret1
                  << std::endl;
        return ret1;
    }

    std::cout << "running: " << std::quoted(cmd_rastertokpsl) << std::endl;
    int ret2 = std::system(cmd_rastertokpsl.c_str());
    if (ret2 != 0)
    {
        std::cerr << "error: rastertokpsl-bin failed with code " << ret2
                  << std::endl;
        return ret2;
    }

    if (!fs::exists(output_file))
    {
        std::cerr << "error: output file " << std::quoted(output_file)
                  << " not found" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "test completed successfully, output file: "
              << std::quoted(output_file) << std::endl;
    return std::cout.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}