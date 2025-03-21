#ifndef __BASE_STDARG_H__
#define __BASE_STDARG_H__

/* compiler provides size of save area */
typedef __builtin_va_list va_list;

#define va_start(ap, last) (__builtin_va_start(ap, last))
#define va_arg(ap, type) (__builtin_va_arg(ap, type))
#define va_end(ap) /*nothing*/

#endif // __BASE_STDARG_H__
