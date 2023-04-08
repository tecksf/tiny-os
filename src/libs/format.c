#include "format.h"
#include "string.h"
#include <x86.h>
#include <error.h>
#include <defs.h>

void print_fmt(void (*output)(int, void *), void *putdat, const char *fmt, ...);

static const char *const error_string[MAX_ERROR + 1] = {
        [0]                 NULL,
        [E_UNSPECIFIED]     "unspecified error",
        [E_BAD_PROC]        "bad process",
        [E_INVAL]           "invalid parameter",
        [E_NO_MEM]          "out of memory",
        [E_NO_FREE_PROC]    "out of processes",
        [E_FAULT]           "segmentation fault",
};

static unsigned long long get_uint(va_list *ap, int lflag)
{
    if (lflag >= 2)
    {
        return va_arg(*ap, unsigned long long);
    }
    else if (lflag)
    {
        return va_arg(*ap, unsigned long);
    }
    else
    {
        return va_arg(*ap, unsigned int);
    }
}

static long long get_int(va_list *ap, int lflag)
{
    if (lflag >= 2)
    {
        return va_arg(*ap, long long);
    }
    else if (lflag)
    {
        return va_arg(*ap, long);
    }
    else
    {
        return va_arg(*ap, int);
    }
}

static void print_num(void (*output)(int, void *), void *putdat, unsigned long long num,
                      unsigned base, int width, int padc)
{
    unsigned long long result = num;
    unsigned mod = do_div(result, base);

    // first recursively print all preceding (more significant) digits
    if (num >= base)
    {
        print_num(output, putdat, result, base, width - 1, padc);
    }
    else
    {
        // print any needed pad characters before first digit
        while (--width > 0)
            output(padc, putdat);
    }
    // then print this (the least significant) digit
    output("0123456789abcdef"[mod], putdat);
}

void format_output(void (*output)(int, void *), void *putdat, const char *fmt, va_list ap)
{
    register const char *p;
    register int ch, err;
    unsigned long long num;
    int base, width, precision, lflag, altflag;

    while (1)
    {
        while ((ch = *(unsigned char *) fmt++) != '%')
        {
            if (ch == '\0')
            {
                return;
            }
            output(ch, putdat);
        }

        // Process a %-escape sequence
        char padc = ' ';
        width = precision = -1;
        lflag = altflag = 0;

reswitch:
        switch (ch = *(unsigned char *) fmt++)
        {
            // flag to pad on the right
            case '-':
                padc = '-';
                goto reswitch;

                // flag to pad with 0's instead of spaces
            case '0':
                padc = '0';
                goto reswitch;

                // width field
            case '1' ... '9':
                for (precision = 0;; ++fmt)
                {
                    precision = precision * 10 + ch - '0';
                    ch = *fmt;
                    if (ch < '0' || ch > '9')
                    {
                        break;
                    }
                }
                goto process_precision;

            case '*':
                precision = va_arg(ap, int);
                goto process_precision;

            case '.':
                if (width < 0)
                    width = 0;
                goto reswitch;

            case '#':
                altflag = 1;
                goto reswitch;

            process_precision:
                if (width < 0)
                    width = precision, precision = -1;
                goto reswitch;

                // long flag (doubled for long long)
            case 'l':
                lflag++;
                goto reswitch;

                // character
            case 'c':
                output(va_arg(ap, int), putdat);
                break;

                // error message
            case 'e':
                err = va_arg(ap, int);
                if (err < 0)
                {
                    err = -err;
                }
                if (err > MAX_ERROR || (p = error_string[err]) == NULL)
                {
                    print_fmt(output, putdat, "error %d", err);
                }
                else
                {
                    print_fmt(output, putdat, "%s", p);
                }
                break;

                // string
            case 's':
                if ((p = va_arg(ap, char *)) == NULL)
                {
                    p = "(null)";
                }
                if (width > 0 && padc != '-')
                {
                    for (width -= string_num_length(p, precision); width > 0; width--)
                    {
                        output(padc, putdat);
                    }
                }
                for (; (ch = *p++) != '\0' && (precision < 0 || --precision >= 0); width--)
                {
                    if (altflag && (ch < ' ' || ch > '~'))
                    {
                        output('?', putdat);
                    }
                    else
                    {
                        output(ch, putdat);
                    }
                }
                for (; width > 0; width--)
                {
                    output(' ', putdat);
                }
                break;

                // (signed) decimal
            case 'd':
                num = get_int(&ap, lflag);
                if ((long long) num < 0)
                {
                    output('-', putdat);
                    num = -(long long) num;
                }
                base = 10;
                goto number;

                // unsigned decimal
            case 'u':
                num = get_uint(&ap, lflag);
                base = 10;
                goto number;

                // (unsigned) octal
            case 'o':
                num = get_uint(&ap, lflag);
                base = 8;
                goto number;

                // pointer
            case 'p':
                output('0', putdat);
                output('x', putdat);
                num = (unsigned long long) (uintptr) va_arg(ap, void *);
                base = 16;
                goto number;

                // (unsigned) hexadecimal
            case 'x':
                num = get_uint(&ap, lflag);
                base = 16;
            number:
                print_num(output, putdat, num, base, width, padc);
                break;

                // escaped '%' character
            case '%':
                output(ch, putdat);
                break;

                // unrecognized escape sequence - just print it literally
            default:
                output('%', putdat);
                for (fmt--; fmt[-1] != '%'; fmt--)
                    /* do nothing */;
                break;
        }
    }
}

void print_fmt(void (*output)(int, void *), void *putdat, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    format_output(output, putdat, fmt, ap);
    va_end(ap);
}