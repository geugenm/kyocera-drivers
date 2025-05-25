#ifndef RASTERTOKPSL_RASTERTOKPSL_H
#define RASTERTOKPSL_RASTERTOKPSL_H

#include <cups/raster.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    uint32_t rastertokpsl(cups_raster_t* ras,
                          const char*    user,
                          const char*    job_name,
                          int32_t        copies,
                          const char*    options);

#ifdef __cplusplus
}
#endif

#endif // RASTERTOKPSL_RASTERTOKPSL_H