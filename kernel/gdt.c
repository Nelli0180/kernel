#include <lib/stdint.h>
#include <lib/string.h>
#include <gdt.h>
#include <kheap.h>
#include <lib/stdio.h>
#include <x86.h>

static gdt_entry_t gdt_entries[6];
static gdt_ptr_t gdt_ptr;

static void gdt_flush(void) {
	asm volatile (
		"lgdt %0\n"
		"mov $0x10, %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"mov %%ax, %%ss\n"
		"ljmp $0x08, $flush_cs\n"
		"flush_cs:\n"
		: : "m"(gdt_ptr) : "eax"
    );
}

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	if (num < 0 || num >= 6) {
		printf("GDT: Invalid gate number %d\n", num);
		return;
	}
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_mid = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;
	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access = access;
}

void gdt_init(void) {
	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base  = (uint32_t)&gdt_entries;

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	gdt_set_gate(5, 0, 0, 0, 0);

	gdt_flush();
}