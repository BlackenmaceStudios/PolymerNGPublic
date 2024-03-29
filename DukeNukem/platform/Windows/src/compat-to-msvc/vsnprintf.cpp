/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#define __CRT__NO_INLINE
#include <stdarg.h>
#include <stdlib.h>

extern int __cdecl _vsnprintf(char *, size_t, const char * , va_list);

int __cdecl __ms_vsnprintf (char *  s, size_t n, const char *  format, va_list arg)
{
    return _vsnprintf(s, n, format, arg);
}
int __cdecl __mingw_vsnprintf (char *  s, size_t n, const char *  format, va_list arg)
{
    return _vsnprintf(s, n, format, arg);
}
