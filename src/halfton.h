#ifndef HALFTON_H
#define HALFTON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void halftone_dib_to_dib(uint8_t* planes8,
                             uint8_t* planes,
                             int32_t  width,
                             int32_t  numver,
                             int32_t  contrast,
                             int32_t  brightness);

#ifdef __cplusplus
}
#endif

#endif // HALFTON_H