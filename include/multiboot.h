#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <lib/stdint.h>

#define MULTIBOOT_HEADER_MAGIC  0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

typedef struct {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms[4];
	uint32_t mmap_length;
	uint32_t mmap_addr;
} multiboot_info_t;

typedef struct {
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t type;
} multiboot_mmap_entry_t;

typedef struct {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t cmdline;
	uint32_t reserved;
} multiboot_module_t;

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2

#endif /* MULTIBOOT_H */