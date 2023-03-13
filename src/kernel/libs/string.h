#ifndef __LIBS_STRING_H__
#define __LIBS_STRING_H__

#include <defs.h>

void *memmove(void *dst, const void *src, usize n);
void *memset(void *s, char c, usize n);
void *memcpy(void *dst, const void *src, usize n);
int memcmp(const void *v1, const void *v2, usize n);

usize strlen(const char *s);
usize strnlen(const char *s, usize n);

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, usize len);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, usize n);

char *strchr(const char *s, char c);
char *strfind(const char *s, char c);
long strtol(const char *s, char **endptr, int base);

#endif // __LIBS_STRING_H__