#ifndef PMM_H
#define PMM_H

#include <lib/stdint.h>
#include <multiboot.h>

#define PAGE_SIZE 4096
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

void pmm_init(multiboot_info_t *mb_info, uint32_t kernel_end);
void *pmm_alloc(uint32_t pages);
void pmm_free(void *addr, uint32_t pages);
int pmm_is_reserved(void *addr);
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_free_pages(void);

#endif /* PMM_H */