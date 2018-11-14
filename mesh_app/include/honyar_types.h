
#ifndef _HONYAR_TYPES_H_
#define _HONYAR_TYPES_H_


#include "c_types.h"
#include "osapi.h"

#define HONYAR_ARRAY_SIZE(array)		(sizeof(array) / sizeof((array)[0]))

#if 1
#define hy_printf(fmt, ...)  os_printf(fmt, ##__VA_ARGS__)
#else
#define eg_printf(fmt, ...)
#endif

#define hy_debug(fmt, ...)  \
    do {\
        hy_printf("[DEBUG]: %s(), (line: %d), " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }while(0)
    
#define hy_info(fmt, ...)  \
    do {\
        hy_printf("[INFO]: %s(), (line: %d), " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }while(0)
    
#define hy_error(fmt, ...) \
    do {\
        hy_printf("[ERROR]: %s(), (line: %d), " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    }while(0)
    
#define hy_fatal(fmt, ...) \
    do {\
        hy_printf("[FATAL]: %s(), (line: %d), " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        system_restart(); \
    }while(0)


#endif
