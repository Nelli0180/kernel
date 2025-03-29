#include <lib/stdint.h>
#include <lib/string.h>
#include <tss.h>
#include <gdt.h>
#include <kheap.h>
#include <lib/stdio.h>
#include <x86.h>
#include <panic.h>

static void tss_flush(uint16_t selector) {
	asm volatile ("ltr %0" : : "r"(selector));
}

tss_entry_t *tss_create(uint32_t ss0, uint32_t esp0, uint32_t ss1, uint32_t esp1, uint32_t ss2, uint32_t esp2, int gdt_index) {
	if (gdt_index < 0 || gdt_index >= 6) {
		panic_custom("TSS: Invalid GDT index specified");
	}
	tss_entry_t *tss = (tss_entry_t *)kmalloc(sizeof(tss_entry_t));
	if (!tss) {
		panic_custom("TSS: Failed to allocate memory for TSS");
	}
	if ((uint32_t)tss & 0x3) {
		kfree(tss);
		panic_custom("TSS: Misaligned TSS address detected");
	}

	memset(tss, 0, sizeof(tss_entry_t));

	tss->ss0 = ss0;
	tss->esp0 = esp0;
	tss->ss1 = ss1;
	tss->esp1 = esp1;
	tss->ss2 = ss2;
	tss->esp2 = esp2;

	tss->cs = 0x08;
	tss->ss = 0x10;
	tss->ds = 0x10;
	tss->es = 0x10;
	tss->fs = 0x10;
	tss->gs = 0x10;

	uint32_t base = (uint32_t)tss;
	uint32_t limit = sizeof(tss_entry_t) - 1;
	gdt_set_gate(gdt_index, base, limit, 0x89, 0x00);

	uint16_t selector = (gdt_index << 3) | 0x00;
	tss_flush(selector);

	printf("TSS: Created at 0x%x, GDT index %d, selector 0x%x\n", base, gdt_index, selector);
	printf("  ring 0: ss0=0x%x, esp0=0x%x\n", ss0, esp0);
	printf("  ring 1: ss1=0x%x, esp1=0x%x\n", ss1, esp1);
	printf("  ring 2: ss2=0x%x, esp2=0x%x\n", ss2, esp2);

	return tss;
}

void tss_set_stack(tss_entry_t *tss, uint32_t ring, uint32_t ss, uint32_t esp) {
	if (!tss) {
		printf("TSS: Invalid TSS pointer!\n");
		return;
	}

	switch (ring) {
		case 0:
			tss->ss0 = ss;
			tss->esp0 = esp;
			printf("TSS: Updated ring 0 stack at 0x%x, ss0=0x%x, esp0=0x%x\n", (uint32_t)tss, ss, esp);
			break;
		case 1:
			tss->ss1 = ss;
			tss->esp1 = esp;
			printf("TSS: Updated ring 1 stack at 0x%x, ss1=0x%x, esp1=0x%x\n", (uint32_t)tss, ss, esp);
			break;
		case 2:
			tss->ss2 = ss;
			tss->esp2 = esp;
			printf("TSS: Updated ring 2 stack at 0x%x, ss2=0x%x, esp2=0x%x\n", (uint32_t)tss, ss, esp);
			break;
		default:
			printf("TSS: Unsupported ring %d\n", ring);
			break;
	}
}

void tss_free(tss_entry_t *tss) {
	if (!tss) {
		printf("TSS: Invalid TSS pointer for free!\n");
		return;
	}
	kfree(tss);
	printf("TSS: Freed at 0x%x\n", (uint32_t)tss);
}