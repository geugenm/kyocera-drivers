#pragma once

/// @file rastertokpsl.hxx
///
/// @brief This header file defines the interface for converting raster data
/// to Kyocera KPSL (Kyocera Printer Command Language) format.
///
/// This library is intended for use in CUPS (Common UNIX Printing System)
/// filters and provides functions to process raster data from CUPS,
/// convert it to KPSL, and handle printing operations specific to Kyocera
/// printers.

#include <cstdint>
#include <string_view>

#include <cups/raster.h>

[[nodiscard]] std::size_t rastertokpsl(cups_raster_t*         raster_stream,
                                       const std::string_view user_name,
                                       const std::string_view job_title,
                                       const std::uint32_t    copies_number,
                                       const std::string_view printing_options);