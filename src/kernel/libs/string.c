#include "x86.h"
#include "string.h"

void *memmove(void *dst, const void *src, usize n)
{
#ifdef __HAVE_ARCH_MEMMOVE
    return __memmove(dst, src, n);
#else
    const char *s = src;
    char *d = dst;
    if (s < d && s + n > d)
    {
        s += n, d += n;
        while (n-- > 0)
        {
            *--d = *--s;
        }
    }
    else
    {
        while (n-- > 0)
        {
            *d++ = *s++;
        }
    }
    return dst;
#endif /* __HAVE_ARCH_MEMMOVE */
}

void *memset(void *s, char c, usize n)
{
    char *p = s;
    while (n-- > 0)
    {
        *p++ = c;
    }
    return s;
}

void *memcpy(void *dst, const void *src, usize n)
{
    const char *s = src;
    char *d = dst;
    while (n-- > 0)
    {
        *d++ = *s++;
    }
    return dst;
}

int memcmp(const void *v1, const void *v2, usize n)
{
    const char *s1 = (const char *) v1;
    const char *s2 = (const char *) v2;
    while (n-- > 0)
    {
        if (*s1 != *s2)
        {
            return (int) ((unsigned char) *s1 - (unsigned char) *s2);
        }
        s1++, s2++;
    }
    return 0;
}

usize strlen(const char *s)
{
    usize len = 0;
    while (*s++ != '\0')
    {
        len++;
    }
    return len;
}

usize strnlen(const char *s, usize n)
{
    usize len = 0;
    while (len < n && *s++ != '\0')
    {
        len++;
    }
    return len;
}

char *strcpy(char *dst, const char *src)
{
    char *p = dst;
    while ((*p++ = *src++) != '\0')
        /* nothing */;
    return dst;
}

char *strncpy(char *dst, const char *src, usize len)
{
    char *p = dst;
    while (len > 0)
    {
        if ((*p = *src) != '\0')
        {
            src++;
        }
        p++, len--;
    }
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s1 == *s2)
    {
        s1++, s2++;
    }
    return (int) ((unsigned char) *s1 - (unsigned char) *s2);
}

int strncmp(const char *s1, const char *s2, usize n)
{
    while (n > 0 && *s1 != '\0' && *s1 == *s2)
    {
        n--, s1++, s2++;
    }
    return (n == 0) ? 0 : (int) ((unsigned char) *s1 - (unsigned char) *s2);
}

char *strchr(const char *s, char c)
{
    while (*s != '\0')
    {
        if (*s == c)
        {
            return (char *) s;
        }
        s++;
    }
    return NULL;
}

char *strfind(const char *s, char c)
{
    while (*s != '\0')
    {
        if (*s == c)
        {
            break;
        }
        s++;
    }
    return (char *) s;
}

long strtol(const char *s, char **endptr, int base)
{
    int neg = 0;
    long val = 0;

    // gobble initial whitespace
    while (*s == ' ' || *s == '\t')
    {
        s++;
    }

    // plus/minus sign
    if (*s == '+')
    {
        s++;
    }
    else if (*s == '-')
    {
        s++, neg = 1;
    }

    // hex or octal base prefix
    if ((base == 0 || base == 16) && (s[0] == '0' && s[1] == 'x'))
    {
        s += 2, base = 16;
    }
    else if (base == 0 && s[0] == '0')
    {
        s++, base = 8;
    }
    else if (base == 0)
    {
        base = 10;
    }

    // digits
    while (1)
    {
        int dig;

        if (*s >= '0' && *s <= '9')
        {
            dig = *s - '0';
        }
        else if (*s >= 'a' && *s <= 'z')
        {
            dig = *s - 'a' + 10;
        }
        else if (*s >= 'A' && *s <= 'Z')
        {
            dig = *s - 'A' + 10;
        }
        else
        {
            break;
        }
        if (dig >= base)
        {
            break;
        }
        s++, val = (val * base) + dig;
        // we don't properly detect overflow!
    }

    if (endptr)
    {
        *endptr = (char *) s;
    }
    return (neg ? -val : val);
}