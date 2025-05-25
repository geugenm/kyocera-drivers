#ifndef RASTERTOKPSL_RASTERTOKPSL_H
#define RASTERTOKPSL_RASTERTOKPSL_H

#include <cups/raster.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    uint32_t rastertokpsl(cups_raster_t* raster_stream,
                          const char*    user_name,
                          const char*    job_title,
                          int32_t        num_copies,
                          const char*    options_str);

#ifdef __cplusplus
}
#endif

#endif // RASTERTOKPSL_RASTERTOKPSL_H