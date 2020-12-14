
// config.h - Define features and OS portability macros

#ifndef _config_h_
#define _config_h_

// TODO: check if this is still needed and/or could be moved to individual
// headers.
#if defined(__APPLE__)
#include <sys/uio.h>
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
