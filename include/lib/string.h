#ifndef STRING_H
#define STRING_H

#include <lib/stddef.h>
#include <lib/stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

void *memcpy_volatile(volatile void *dest, const volatile void *src, size_t n);
void *memmove_volatile(volatile void *dest, const volatile void *src, size_t n);
void *memset_volatile(volatile void *s, int c, size_t n);

size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);

int atoi(const char *str);
uint32_t atox(const char *str);

#endif /* STRING_H */