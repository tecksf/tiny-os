#ifndef __USER_FORMAT_H__
#define __USER_FORMAT_H__

#include <stdarg.h>

void format_output(void (*output)(int, void *), void *putdat, const char *fmt, va_list ap);

#endif // __USER_FORMAT_H__
