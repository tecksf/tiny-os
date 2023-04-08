#ifndef __LIBS_STRING_H__
#define __LIBS_STRING_H__

#include <defs.h>

void *memory_move(void *dst, const void *src, usize n);
void *memory_set(void *s, char c, usize n);
void *memory_copy(void *dst, const void *src, usize n);
int memory_compare(const void *v1, const void *v2, usize n);

usize string_length(const char *s);
usize string_num_length(const char *s, usize n);

char *string_copy(char *dst, const char *src);
char *string_num_copy(char *dst, const char *src, usize len);

int string_compare(const char *s1, const char *s2);
int string_num_compare(const char *s1, const char *s2, usize n);

char *string_char_retrieve(const char *s, char c);
char *string_find(const char *s, char c);
long string_to_long(const char *s, char **endptr, int base);

#endif // __LIBS_STRING_H__