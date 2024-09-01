#pragma once

/// @file rastertokpsl.hxx
///
/// @brief This header file defines the interface for converting raster data
/// to HP PCL-XL (Printer Command Language - XL) format.
///
/// This library is intended for use in CUPS (Common UNIX Printing System)
/// filters and provides functions to process raster data from CUPS,
/// convert it to PCL-XL, and handle printing operations.

#include <cstdio>
#include <cwchar>

[[nodiscard]] std::size_t rastertokpsl(cups_raster_t* raster_stream,
                                       std::string_view user_name,
                                       std::string_view job_title,
                                       uint32_t         copies_number,
                                       std::string_view printing_options);