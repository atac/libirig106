
// config.h - Define features and OS portability macros

#ifndef _config_h_
#define _config_h_

/* #if defined(__GNUC__) */
/* #define PACKED __attribute__ ((packed)) */
/* #else */
#define PACKED
/* #endif */

#if defined(__APPLE__)
#include <sys/uio.h>
#endif

// .NET 2005 (and probably earlier, but I'm not sure) define time_t to be a 64 bit value.
// And by default, all the CRT time routines are the 64 bit versions.  For best portability,
// time_t is assumed to be a 32 bit value.  The following #define tells .NET to use 32 bits
// as the default time_t size.  This needs to be set in the project properties.  This forces
// a puke if it isn't set.
#if _MSC_VER >= 1400
#if !defined(_USE_32BIT_TIME_T)
#pragma message("WARNING - '_USE_32BIT_TIME_T' not set!")
#endif
#endif

// The POSIX caseless string compare is strcasecmp(). MSVC uses the
// non-standard stricmp(). Fix it up with a macro if necessary

#if defined(_MSC_VER)
#define strcasecmp(s1, s2)          _stricmp(s1, s2)
#define strncasecmp(s1, s2, n)      _strnicmp(s1, s2, n)
#pragma warning(disable : 4996)
#endif


// File open flags

// Microsoft
#if defined(_MSC_VER)
#define READ_FLAGS O_RDONLY | O_BINARY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT | _O_TRUNC | O_BINARY
#define OVERWRITE_MODE _S_IREAD | _S_IWRITE

// GCC
#elif defined(__GNUC__)
#define OVERWRITE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#if !defined(__APPLE__)
#define READ_FLAGS O_RDONLY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT

// OSX
#else
#define READ_FLAGS O_RDONLY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT
#endif

// Everyone else
#else
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT
#define OVERWRITE_MODE 0
#endif


#endif
