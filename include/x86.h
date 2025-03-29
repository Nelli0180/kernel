#ifndef X86_H
#define X86_H

#include <lib/stdint.h>

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
	asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
	asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void cli(void) {
	asm volatile ("cli");
}

static inline void sti(void) {
	asm volatile ("sti");
}

static inline void hlt(void) {
	asm volatile ("hlt");
}

static inline void io_wait(void) {
	outb(0x80, 0);
}

#endif /* X86_H */