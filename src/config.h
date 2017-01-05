
// config.h - Define features and OS portability macros

#ifndef _config_h_
#define _config_h_

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLIC

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

// .NET managed code extends good ol' Stroustrup C++ in some interesting and unique ways.
// I don't know what Bjarne would say, but here in the real world we need to deal with it.
#if defined(_M_CEE)
#define PUBLIC_CLASS    public

#else
#define PUBLIC_CLASS

#endif

/* The POSIX caseless string compare is strcasecmp(). MSVC uses the
 * non-standard stricmp(). Fix it up with a macro if necessary
 */

#if defined(_MSC_VER)
#define strcasecmp(s1, s2)          _stricmp(s1, s2)
#define strncasecmp(s1, s2, n)      _strnicmp(s1, s2, n)
#pragma warning(disable : 4996)
#endif

#define I106_CALL_DECL

// Turn on network support
// #define IRIG_NETWORKING

#ifdef __cplusplus
}
#endif

#endif
