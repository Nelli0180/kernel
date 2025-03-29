#include <lib/stddef.h>
#include <lib/stdint.h>
#include <lib/stdarg.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <keyboard.h>
#include <x86.h>

static volatile uint16_t *vga_buffer = (volatile uint16_t *)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen(void) {
    volatile uint16_t *p = vga_buffer;
    uint16_t blank = ' ' | (0x07 << 8);
    for (int i = VGA_WIDTH * VGA_HEIGHT / 4; i > 0; i--) {
        *p++ = blank;
        *p++ = blank;
        *p++ = blank;
        *p++ = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor(cursor_y, cursor_x);
}

void scroll_screen(void) {
    memmove_volatile(vga_buffer, vga_buffer + VGA_WIDTH, VGA_WIDTH * (VGA_HEIGHT - 1) * 2);
    volatile uint16_t *last_line = vga_buffer + VGA_WIDTH * (VGA_HEIGHT - 1);
    uint16_t blank = ' ' | (0x07 << 8);
    for (int i = VGA_WIDTH / 4; i > 0; i--) {
        *last_line++ = blank;
        *last_line++ = blank;
        *last_line++ = blank;
        *last_line++ = blank;
    }
    cursor_y--;
    update_cursor(cursor_y, cursor_x);
}

void update_cursor(int row, int col) {
    uint16_t pos = row * VGA_WIDTH + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = ' ' | (0x07 << 8);
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = ' ' | (0x07 << 8);
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = c | (0x07 << 8);
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT) scroll_screen();
    update_cursor(cursor_y, cursor_x);
}

void puts(const char *s) {
    while (*s) putchar(*s++);
}

static void itoa(int value, char *buf) {
    const char *digits = "0123456789";
    char temp[12];
    int i = 0;
    int is_negative = value < 0;

    if (is_negative) value = -value;
    do {
        temp[i++] = digits[value % 10];
        value /= 10;
    } while (value);
    if (is_negative) temp[i++] = '-';

    char *p = buf;
    while (i) *p++ = temp[--i];
    *p = '\0';
}

static void utoa(uint32_t value, char *buf) {
    const char *digits = "0123456789abcdef";
    char temp[8];
    int i = 0;

    do {
        temp[i++] = digits[value % 16];
        value /= 16;
    } while (value);
    char *p = buf;
    while (i) *p++ = temp[--i];
    *p = '\0';
}

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buf[12];

    while (*format) {
        if (*format == '%') {
            format++;
            int zero_pad = *format == '0';
            if (zero_pad) format++;
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format++ - '0');
            }

            if (*format == 's') {
                const char *s = va_arg(args, const char *);
                while (*s) putchar(*s++);
            } else if (*format == 'd') {
                itoa(va_arg(args, int), buf);
                int len = strlen(buf);
                if (zero_pad && width > len) while (width-- > len) putchar('0');
                puts(buf);
            } else if (*format == 'x') {
                utoa(va_arg(args, uint32_t), buf);
                int len = strlen(buf);
                if (zero_pad && width > len) while (width-- > len) putchar('0');
                puts(buf);
            } else {
                putchar('%');
                if (*format) putchar(*format);
            }
        } else {
            putchar(*format);
        }
        format++;
    }
    va_end(args);
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buf[12];
    size_t pos = 0;

    while (*format && pos < size - 1) {
        if (*format == '%') {
            format++;
            int zero_pad = *format == '0';
            if (zero_pad) format++;
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format++ - '0');
            }

            if (*format == 's') {
                const char *s = va_arg(args, const char *);
                while (*s && pos < size - 1) {
                    str[pos++] = *s++;
                }
            } else if (*format == 'd') {
                itoa(va_arg(args, int), buf);
                int len = strlen(buf);
                if (zero_pad && width > len) while (width-- > len && pos < size - 1) str[pos++] = '0';
                char *p = buf;
                while (*p && pos < size - 1) str[pos++] = *p++;
            } else if (*format == 'x') {
                utoa(va_arg(args, uint32_t), buf);
                int len = strlen(buf);
                if (zero_pad && width > len) while (width-- > len && pos < size - 1) str[pos++] = '0';
                char *p = buf;
                while (*p && pos < size - 1) str[pos++] = *p++;
            } else {
                str[pos++] = '%';
                if (*format && pos < size - 1) str[pos++] = *format;
            }
        } else {
            str[pos++] = *format;
        }
        format++;
    }
    str[pos] = '\0';
    va_end(args);
    return pos;
}

char getchar(void) {
    return keyboard_getc();
}

char *gets(char *str, size_t max_len) {
    char *p = str;
    size_t len = 0;
    if (!max_len) return str;

    while (1) {
        char c = getchar();
        if (c == '\n') {
            putchar('\n');
            break;
        }
        if (c == '\b') {
            if (len) {
                p--;
                len--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            continue;
        }
        if (len < max_len - 1) {
            *p++ = c;
            len++;
            putchar(c);
        }
    }
    *p = '\0';
    return str;
}