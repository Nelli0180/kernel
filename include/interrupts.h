#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <lib/stdint.h>
#include <panic.h>

#define IDT_ENTRIES 256
#define IDT_GATE_INT32 0x8E

typedef struct {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idt_ptr_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void irq0(void);
extern void irq1(void);

#endif /* INTERRUPTS_H */
