#include <lib/string.h>
#include <lib/stdint.h>

void *memcpy(void *dest, const void *src, size_t n) {
	if (n == 0) return dest;

	void *ret = dest;

	while (n > 0 && ((uint32_t)dest & 3)) {
		*(uint8_t *)dest = *(const uint8_t *)src;
		dest = (uint8_t *)dest + 1;
		src = (const uint8_t *)src + 1;
		n--;
	}

	if (n >= 4) {
		size_t count = n / 4;
		asm volatile (
			"cld\n"
			"rep movsl\n"
			: "+D"(dest), "+S"(src), "+c"(count)
			: 
			: "memory"
		);
		n %= 4;
	}

	while (n > 0) {
		*(uint8_t *)dest = *(const uint8_t *)src;
		dest = (uint8_t *)dest + 1;
		src = (const uint8_t *)src + 1;
		n--;
	}

	return ret;
}

void *memmove(void *dest, const void *src, size_t n) {
	if (n == 0) return dest;

	void *ret = dest;

	if (dest <= src || (uint8_t *)dest >= (uint8_t *)src + n) {
		while (n > 0 && ((uint32_t)dest & 3)) {
			*(uint8_t *)dest = *(const uint8_t *)src;
			dest = (uint8_t *)dest + 1;
			src = (const uint8_t *)src + 1;
			n--;
		}

		if (n >= 4) {
			size_t count = n / 4;
			asm volatile (
				"cld\n"
				"rep movsl\n"
				: "+D"(dest), "+S"(src), "+c"(count)
				: 
				: "memory"
			);
			n %= 4;
		}

		while (n > 0) {
			*(uint8_t *)dest = *(const uint8_t *)src;
			dest = (uint8_t *)dest + 1;
			src = (const uint8_t *)src + 1;
			n--;
		}
	} else {
		dest = (uint8_t *)dest + n;
		src = (const uint8_t *)src + n;

		while (n > 0 && ((uint32_t)dest & 3)) {
			dest = (uint8_t *)dest - 1;
			src = (const uint8_t *)src - 1;
			*(uint8_t *)dest = *(const uint8_t *)src;
			n--;
		}

		if (n >= 4) {
			size_t count = n / 4;
			asm volatile (
				"std\n"
				"rep movsl\n"
				"cld\n"
				: "+D"(dest), "+S"(src), "+c"(count)
				: 
				: "memory"
			);
			n %= 4;
		}

		while (n > 0) {
			dest = (uint8_t *)dest - 1;
			src = (const uint8_t *)src - 1;
			*(uint8_t *)dest = *(const uint8_t *)src;
			n--;
		}
	}

	return ret;
}

void *memset(void *s, int c, size_t n) {
	if (n == 0) return s;

	void *ret = s;
	uint8_t val = (uint8_t)c;
	uint32_t val32 = val * 0x01010101;

	while (n > 0 && ((uint32_t)s & 3)) {
		*(uint8_t *)s = val;
		s = (uint8_t *)s + 1;
		n--;
	}

	if (n >= 4) {
		size_t count = n / 4;
		asm volatile (
			"cld\n"
			"rep stosl\n"
			: "+D"(s), "+c"(count)
			: "a"(val32)
			: "memory"
		);
		n %= 4;
	}

	while (n > 0) {
		*(uint8_t *)s = val;
		s = (uint8_t *)s + 1;
		n--;
	}

	return ret;
}

void *memcpy_volatile(volatile void *dest, const volatile void *src, size_t n) {
	if (n == 0) return (void *)dest;

	void *ret = (void *)dest;

	while (n > 0 && ((uint32_t)dest & 3)) {
		*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
		dest = (volatile uint8_t *)dest + 1;
		src = (const volatile uint8_t *)src + 1;
		n--;
	}

	if (n >= 4) {
		size_t count = n / 4;
		asm volatile (
			"cld\n"
			"rep movsl\n"
			: "+D"(dest), "+S"(src), "+c"(count)
			: 
			: "memory"
		);
		n %= 4;
	}

	while (n > 0) {
		*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
		dest = (volatile uint8_t *)dest + 1;
		src = (const volatile uint8_t *)src + 1;
		n--;
	}

	return ret;
}

void *memmove_volatile(volatile void *dest, const volatile void *src, size_t n) {
	if (n == 0) return (void *)dest;

	void *ret = (void *)dest;

	if (dest <= src || (volatile uint8_t *)dest >= (volatile uint8_t *)src + n) {
		while (n > 0 && ((uint32_t)dest & 3)) {
			*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
			dest = (volatile uint8_t *)dest + 1;
			src = (const volatile uint8_t *)src + 1;
			n--;
		}

		if (n >= 4) {
			size_t count = n / 4;
			asm volatile (
				"cld\n"
				"rep movsl\n"
				: "+D"(dest), "+S"(src), "+c"(count)
				: 
				: "memory"
			);
			n %= 4;
		}

		while (n > 0) {
			*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
			dest = (volatile uint8_t *)dest + 1;
			src = (const volatile uint8_t *)src + 1;
			n--;
		}
	} else {
		dest = (volatile uint8_t *)dest + n;
		src = (const volatile uint8_t *)src + n;

		while (n > 0 && ((uint32_t)dest & 3)) {
			dest = (volatile uint8_t *)dest - 1;
			src = (const volatile uint8_t *)src - 1;
			*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
			n--;
		}

		if (n >= 4) {
			size_t count = n / 4;
			asm volatile (
				"std\n"
				"rep movsl\n"
				"cld\n"
				: "+D"(dest), "+S"(src), "+c"(count)
				: 
				: "memory"
			);
			n %= 4;
		}

		while (n > 0) {
			dest = (volatile uint8_t *)dest - 1;
			src = (const volatile uint8_t *)src - 1;
			*(volatile uint8_t *)dest = *(const volatile uint8_t *)src;
			n--;
		}
	}

	return ret;
}

void *memset_volatile(volatile void *s, int c, size_t n) {
	if (n == 0) return (void *)s;

	void *ret = (void *)s;
	uint8_t val = (uint8_t)c;
	uint32_t val32 = val * 0x01010101;

	while (n > 0 && ((uint32_t)s & 3)) {
		*(volatile uint8_t *)s = val;
		s = (volatile uint8_t *)s + 1;
		n--;
	}

	if (n >= 4) {
		size_t count = n / 4;
		asm volatile (
			"cld\n"
			"rep stosl\n"
			: "+D"(s), "+c"(count)
			: "a"(val32)
			: "memory"
		);
		n %= 4;
	}

	while (n > 0) {
		*(volatile uint8_t *)s = val;
		s = (volatile uint8_t *)s + 1;
		n--;
	}

	return ret;
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
