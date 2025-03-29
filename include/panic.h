#ifndef PANIC_H
#define PANIC_H

#include <lib/stdint.h>

typedef struct {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void panic(registers_t *regs);
void panic_custom(const char *message);

#endif /* PANIC_H */