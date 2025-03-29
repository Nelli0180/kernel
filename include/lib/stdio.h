#ifndef STDIO_H
#define STDIO_H

#include <lib/stdarg.h>
#include <lib/stddef.h>

void clear_screen(void);
void puts(const char *s);
void printf(const char *format, ...);
void putchar(char c);
void scroll_screen(void);
void update_cursor(int row, int col);
char *gets(char *str, size_t max_len);
char getchar(void);
int snprintf(char *str, size_t size, const char *format, ...);

#endif /* STDIO_H */