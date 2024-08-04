#pragma once

#include "stdlib.h"

void halftone_dib_to_dib(unsigned char* planes8,
                         unsigned char* planes,
                         int            width,
                         int            numver,
                         int            contrast,
                         int            brightness);

void set_default_screen();
unsigned char * get_dither_table();