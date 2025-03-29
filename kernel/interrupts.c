#include <lib/stdint.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <x86.h>
#include <interrupts.h>
#include <panic.h>
#include <keyboard.h>
#include <timer.h>

static idt_entry_t idt_entries[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	idt_entries[num].offset_low  = base & 0xFFFF;
	idt_entries[num].offset_high = (base >> 16) & 0xFFFF;
	idt_entries[num].selector    = sel;
	idt_entries[num].zero        = 0;
	idt_entries[num].type_attr   = flags;
}

extern void isr0(void); extern void isr1(void); extern void isr2(void); extern void isr3(void);
extern void isr4(void); extern void isr5(void); extern void isr6(void); extern void isr7(void);
extern void isr8(void); extern void isr9(void); extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

void idt_init(void) {
	memset(idt_entries, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

	idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
	idt_ptr.base  = (uint32_t)&idt_entries;

    static void (*isr_handlers[])() = {
		isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
		isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
		isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
		isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };
	for (int i = 0; i < 32; i++) {
		idt_set_gate(i, (uint32_t)isr_handlers[i], 0x08, IDT_GATE_INT32);
    }

	idt_set_gate(32, (uint32_t)irq0, 0x08, IDT_GATE_INT32);
	idt_set_gate(33, (uint32_t)irq1, 0x08, IDT_GATE_INT32);

	asm volatile ("lidt %0" : : "m"(idt_ptr));
	printf("IDT: Initialized with %d entries at 0x%x\n", IDT_ENTRIES, (uint32_t)&idt_entries);
}

#define ISR_NOERR(n) \
	void isr##n(void) { \
		asm volatile ( \
			"pushl $0\n" \
			"pushl $" #n "\n" \
			"jmp isr_common\n" \
		); \
	}

#define ISR_ERR(n) \
	void isr##n(void) { \
		asm volatile ( \
			"pushl $" #n "\n" \
			"jmp isr_common\n" \
		); \
	}

__attribute__((naked))
void isr_common(void) {
	asm volatile (
		"pusha\n"
		"mov %ds, %eax\n"
		"push %eax\n"
		"mov $0x10, %ax\n"
		"mov %ax, %ds\n"
		"mov %ax, %es\n"
		"mov %ax, %fs\n"
		"mov %ax, %gs\n"
		"mov %esp, %eax\n"
		"push %eax\n"
		"call panic\n"
		"add $4, %esp\n"
		"pop %eax\n"
		"mov %ax, %ds\n"
		"mov %ax, %es\n"
		"mov %ax, %fs\n"
		"mov %ax, %gs\n"
		"popa\n"
		"add $8, %esp\n"
		"iret\n"
	);
}

__attribute__((naked))
void irq0(void) {
    asm volatile (
		"cli\n"
		"pusha\n"
		"mov $0x10, %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"push $0\n"
		"call pic_send_eoi\n"
		"add $4, %%esp\n"
		"call timer_interrupt_handler\n"
		"popa\n"
		"sti\n"
		"iret\n"
		:
		:
		: "eax"
	);
}

__attribute__((naked))
void irq1(void) {
	asm volatile (
		"cli\n"                   // Отключаем прерывания
		"pusha\n"
		"mov $0x10, %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"call keyboard_interrupt_handler\n"
		"popa\n"
		"sti\n"                   // Включаем прерывания
		"iret\n"
		:
		:
		: "eax"
	);
}

ISR_NOERR(0) ISR_NOERR(1) ISR_NOERR(2) ISR_NOERR(3) ISR_NOERR(4) ISR_NOERR(5) ISR_NOERR(6) ISR_NOERR(7)
ISR_ERR(8) ISR_NOERR(9) ISR_ERR(10) ISR_ERR(11) ISR_ERR(12) ISR_ERR(13) ISR_ERR(14) ISR_NOERR(15)
ISR_NOERR(16) ISR_ERR(17) ISR_NOERR(18) ISR_NOERR(19) ISR_NOERR(20) ISR_ERR(21) ISR_NOERR(22) ISR_NOERR(23)
ISR_NOERR(24) ISR_NOERR(25) ISR_NOERR(26) ISR_NOERR(27) ISR_NOERR(28) ISR_NOERR(29) ISR_ERR(30) ISR_NOERR(31)