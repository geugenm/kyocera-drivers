#pragma once

#include "stdlib.h"

void halftone_dib_to_dib(unsigned char* source_planes8,
                         unsigned char* destination_planes,
                         size_t         image_width,
                         size_t         num_rows,
                         int            contrast,
                         int            brightness);

void set_default_screen();
unsigned char * get_current_dither_table();