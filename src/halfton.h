#pragma once

#include "stdlib.h"

void halftone_dib_to_dib(unsigned char *planes8, unsigned char *planes,
                         int width, int numver, int contrast, int brightness);