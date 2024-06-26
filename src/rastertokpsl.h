#pragma once

/// @file rastertokpsl.h
///
/// @brief This header file defines the interface for converting raster data
/// to HP PCL-XL (Printer Command Language - XL) format.
///
/// This library is intended for use in CUPS (Common UNIX Printing System)
/// filters and provides functions to process raster data from CUPS,
/// convert it to PCL-XL, and handle printing operations.

#include <stdio.h>
#include <wchar.h>

#include <cups/versioning.h>

// <cups/language-private.h>
extern int _cupsLangPrintFilter(FILE *file_to_write_to, const char *non_localized_msg_prefix,
                                const char *message, ...) _CUPS_FORMAT(3, 4) _CUPS_PRIVATE;
// end <cups/language-private.h>

void start_page(cups_page_header2_t *page_header);
void end_page(int section_end);
void shutdown_printer();

void cancel_job(int signal);

int rastertokpsl(cups_raster_t *raster_stream, const char *user_name, const char *job_title,
                 int copies_number, const char *printing_options);