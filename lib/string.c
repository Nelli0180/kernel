#include <lib/string.h>
#include <lib/stdint.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    while (n > 0 && (((uint32_t)d & 3) != 0)) {
        *d++ = *s++;
        n--;
    }

    uint32_t *d32 = (uint32_t *)d;
    const uint32_t *s32 = (const uint32_t *)s;
    while (n >= 4) {
        *d32++ = *s32++;
        n -= 4;
    }

    d = (uint8_t *)d32;
    s = (uint8_t *)s32;
    while (n > 0) {
        *d++ = *s++;
        n--;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d < s) {
        while (n > 0 && (((uint32_t)d & 3) != 0)) {
            *d++ = *s++;
            n--;
        }
        uint32_t *d32 = (uint32_t *)d;
        const uint32_t *s32 = (const uint32_t *)s;
        while (n >= 4) {
            *d32++ = *s32++;
            n -= 4;
        }
        d = (uint8_t *)d32;
        s = (uint8_t *)s32;
        while (n > 0) {
            *d++ = *s++;
            n--;
        }
    } else {
        d += n;
        s += n;
        while (n > 0 && (((uint32_t)d & 3) != 0)) {
            *--d = *--s;
            n--;
        }
        uint32_t *d32 = (uint32_t *)d;
        const uint32_t *s32 = (const uint32_t *)s;
        while (n >= 4) {
            *--d32 = *--s32;
            n -= 4;
        }
        d = (uint8_t *)d32;
        s = (uint8_t *)s32;
        while (n > 0) {
            *--d = *--s;
            n--;
        }
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    uint8_t val = (uint8_t)c;
    uint32_t val32 = (val << 24) | (val << 16) | (val << 8) | val;

    while (n > 0 && (((uint32_t)p & 3) != 0)) {
        *p++ = val;
        n--;
    }

    uint32_t *p32 = (uint32_t *)p;
    while (n >= 4) {
        *p32++ = val32;
        n -= 4;
    }

    p = (uint8_t *)p32;
    while (n > 0) {
        *p++ = val;
        n--;
    }
    return s;
}

void *memcpy_volatile(volatile void *dest, const volatile void *src, size_t n) {
    volatile uint8_t *d = (volatile uint8_t *)dest;
    const volatile uint8_t *s = (const volatile uint8_t *)src;

    while (n > 0 && (((uint32_t)d & 3) != 0)) {
        *d++ = *s++;
        n--;
    }
    volatile uint32_t *d32 = (volatile uint32_t *)d;
    const volatile uint32_t *s32 = (const volatile uint32_t *)s;
    while (n >= 4) {
        *d32++ = *s32++;
        n -= 4;
    }
    d = (volatile uint8_t *)d32;
    s = (const volatile uint8_t *)s32;
    while (n > 0) {
        *d++ = *s++;
        n--;
    }
    return (void *)dest;
}

void *memmove_volatile(volatile void *dest, const volatile void *src, size_t n) {
    volatile uint8_t *d = (volatile uint8_t *)dest;
    const volatile uint8_t *s = (const volatile uint8_t *)src;

    if (d < s) {
        while (n > 0 && (((uint32_t)d & 3) != 0)) {
            *d++ = *s++;
            n--;
        }
        volatile uint32_t *d32 = (volatile uint32_t *)d;
        const volatile uint32_t *s32 = (const volatile uint32_t *)s;
        while (n >= 4) {
            *d32++ = *s32++;
            n -= 4;
        }
        d = (volatile uint8_t *)d32;
        s = (const volatile uint8_t *)s32;
        while (n > 0) {
            *d++ = *s++;
            n--;
        }
    } else {
        d += n;
        s += n;
        while (n > 0 && (((uint32_t)d & 3) != 0)) {
            *--d = *--s;
            n--;
        }
        volatile uint32_t *d32 = (volatile uint32_t *)d;
        const volatile uint32_t *s32 = (const volatile uint32_t *)s;
        while (n >= 4) {
            *--d32 = *--s32;
            n -= 4;
        }
        d = (volatile uint8_t *)d32;
        s = (const volatile uint8_t *)s32;
        while (n > 0) {
            *--d = *--s;
            n--;
        }
    }
    return (void *)dest;
}

void *memset_volatile(volatile void *s, int c, size_t n) {
    volatile uint8_t *p = (volatile uint8_t *)s;
    uint8_t val = (uint8_t)c;
    uint32_t val32 = (val << 24) | (val << 16) | (val << 8) | val;

    while (n > 0 && (((uint32_t)p & 3) != 0)) {
        *p++ = val;
        n--;
    }
    volatile uint32_t *p32 = (volatile uint32_t *)p;
    while (n >= 4) {
        *p32++ = val32;
        n -= 4;
    }
    p = (volatile uint8_t *)p32;
    while (n > 0) {
        *p++ = val;
        n--;
    }
    return (void *)s;
}

size_t strlen(const char *s) {
    const char *start = s;
    while (*s) s++;
    return s - start;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return n == (size_t)-1 ? 0 : *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    while (n--) *d++ = '\0';
    return dest;
}

int atoi(const char *str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str++ - '0');
    }
    return result;
}

uint32_t atox(const char *str) {
    uint32_t result = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    while (*str) {
        if (*str >= '0' && *str <= '9') result = result * 16 + (*str - '0');
        else if (*str >= 'a' && *str <= 'f') result = result * 16 + (*str - 'a' + 10);
        else if (*str >= 'A' && *str <= 'F') result = result * 16 + (*str - 'A' + 10);
        else break;
        str++;
    }
    return result;
}
