//This section is comprised of typedefs from the Espressif ESP8266 SDK
//typedef unsigned char       uint8_t;
typedef signed char         sint8_t;
//typedef signed char         int8_t;
//typedef unsigned short      uint16_t;
typedef signed short        sint16_t;
//typedef signed short        int16_t;
//typedef unsigned long       uint32_t;
typedef signed long         sint32_t;
//typedef signed long         int32_t;
typedef signed long long    sint64_t;
//typedef unsigned long long  uint64_t;
typedef unsigned long long  u_int64_t;
typedef float               real32_t;
typedef double              real64_t;

typedef unsigned char       uint8;
typedef unsigned char       u8;
typedef signed char         sint8;
typedef signed char         int8;
typedef signed char         s8;
typedef unsigned short      uint16;
typedef unsigned short      u16;
typedef signed short        sint16;
typedef signed short        s16;
typedef unsigned int        uint32;
typedef unsigned int        u_int;
typedef unsigned int        u32;
typedef signed int          sint32;
typedef signed int          s32;
typedef int                 int32;
typedef signed long long    sint64;
typedef unsigned long long  uint64;
typedef unsigned long long  u64;
typedef float               real32;
typedef double              real64;


#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

//This section allows us to ignore ICACHE_FLASH_ATTR when compiling for Linux
#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR