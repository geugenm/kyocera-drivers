#include <array>
#include <cassert>
#include <iostream>
#include <string_view>

extern "C"
{
#include "ConvertUTF.h"
}

int convert_text(std::string_view text,
                 int              buffer_length,
                 std::string_view message)
{
    std::array<UTF16, 64> buffer{};

    const UTF8*            source_to_start = (const UTF8*)(text.data());
    UTF16*                 target_start    = (UTF16*)&buffer;
    const ConversionResult conversion_result =
        ConvertUTF8toUTF16(&source_to_start,
                           source_to_start + text.length(),
                           &target_start,
                           target_start + buffer_length,
                           strictConversion);

    std::cout << "Info " << message
              << ": conversion result: " << conversion_result << std::endl;
    return conversion_result;
}

void test_unicode()
{
    const int32_t normal_ascii_convert =
        convert_text("example", 8, "normal ascii");
    assert(normal_ascii_convert == 0);

    const int32_t source_is_larger_than_the_target =
        convert_text("example_123", 8, "ascii source > target");
    assert(source_is_larger_than_the_target == 2);

    const int32_t normal_utf8_convert =
        convert_text("exam_опс", 8, "normal utf8");
    assert(normal_utf8_convert == 0);

    const int32_t utf8_source_is_larger_than_the_target =
        convert_text("exam_опсс", 8, "utf8 source > target");
    assert(utf8_source_is_larger_than_the_target == 2);
}

int main()
{
    test_unicode();
}
