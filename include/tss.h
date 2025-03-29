#ifndef TSS_H
#define TSS_H

#include <lib/stdint.h>

typedef struct {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax, ecx, edx, ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi, edi;
	uint32_t es, cs, ss, ds, fs, gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

tss_entry_t *tss_create(uint32_t ss0, uint32_t esp0, uint32_t ss1, uint32_t esp1, uint32_t ss2, uint32_t esp2, int gdt_index);
void tss_set_stack(tss_entry_t *tss, uint32_t ring, uint32_t ss, uint32_t esp);
void tss_free(tss_entry_t *tss);

#endif /* TSS_H */