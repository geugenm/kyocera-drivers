#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

int main()
{
    namespace fs = std::filesystem;

    const std::string pdf_file    = "test.pdf";
    const std::string ppd_file    = "Kyocera_FS-1020MFPGDI.ppd";
    const std::string output_file = "output.kpsl";
    const std::string log_file    = "cupsfilter.log";

    fs::remove(output_file);
    fs::remove(log_file);

    std::string cmd = "cupsfilter -p " + ppd_file + " -m printer/foo " +
                      pdf_file + " -e > " + output_file + " 2> " + log_file;

    std::cout << "running: " << std::quoted(cmd) << std::endl;
    int ret = std::system(cmd.c_str());
    if (ret != 0)
    {
        std::cerr << "error: cupsfilter failed with code " << ret << std::endl;
        return ret;
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