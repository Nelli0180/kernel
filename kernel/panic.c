#include <lib/stdio.h>
#include <lib/string.h>
#include <panic.h>
#include <x86.h>

static const char *exception_messages[] = {
	"Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
	"Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
	"Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
	"Stack-Segment Fault", "General Protection Fault", "Page Fault", "Reserved (15)",
	"x87 Floating-Point Exception", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception",
	"Virtualization Exception", "Control Protection Exception", "Reserved (22)", "Reserved (23)",
	"Reserved (24)", "Reserved (25)", "Reserved (26)", "Reserved (27)",
	"Hypervisor Injection", "VMM Communication Exception", "Security Exception", "Reserved (31)"
};

void panic(registers_t *regs) {
	cli();
	//clear_screen();

	printf("=== Kernel Panic ===\n");

	if (!regs) {
		printf("Error: Invalid registers pointer!\n");
	} else if (regs->int_no < 32) {
		printf("Exception %d: %s\n", regs->int_no, exception_messages[regs->int_no]);
	} else {
		printf("Unknown interrupt: %d\n", regs->int_no);
	}

	if (regs && (regs->int_no == 8 || (regs->int_no >= 10 && regs->int_no <= 14) || regs->int_no == 17 || regs->int_no == 21 || regs->int_no == 30)) {
		printf("Error code: 0x%x\n", regs->err_code);
	}

	if (regs) {
		printf("\nRegisters:\n");
		printf("EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
		printf("ESI: 0x%x  EDI: 0x%x  EBP: 0x%x  ESP: 0x%x\n", regs->esi, regs->edi, regs->ebp, regs->esp);
		printf("EIP: 0x%x  CS: 0x%x  EFLAGS: 0x%x\n", regs->eip, regs->cs, regs->eflags);
		printf("DS: 0x%x  SS: 0x%x  User ESP: 0x%x\n", regs->ds, regs->ss, regs->useresp);
	}

	while (1) { hlt(); }
}

void panic_custom(const char *message) {
	cli();
	//clear_screen();

	printf("=== Kernel Panic ===\n");
	printf("Error: %s\n", message ? message : "Unknown error");

	while (1) { hlt(); }
}