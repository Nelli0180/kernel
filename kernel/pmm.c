#include <pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <x86.h>

static uint8_t *pmm_bitmap = NULL;
static uint32_t bitmap_size = 0;
static uint32_t total_pages = 0;
static uint32_t free_pages = 0;
static uint32_t memory_base = 0;

static void pmm_set_bit(uint32_t page_idx) {
	uint32_t byte_idx = page_idx / 8;
	uint8_t bit_idx = page_idx % 8;
	pmm_bitmap[byte_idx] |= (1 << bit_idx);
}

static void pmm_clear_bit(uint32_t page_idx) {
	uint32_t byte_idx = page_idx / 8;
	uint8_t bit_idx = page_idx % 8;
	pmm_bitmap[byte_idx] &= ~(1 << bit_idx);
}

static int pmm_test_bit(uint32_t page_idx) {
	uint32_t byte_idx = page_idx / 8;
	uint8_t bit_idx = page_idx % 8;
	return (pmm_bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

static uint32_t pmm_find_free_pages(uint32_t pages) {
	if (pages == 0 || pages > total_pages) {
		return 0xFFFFFFFF;
	}

	uint32_t consecutive = 0;
	for (uint32_t i = 0; i < total_pages; i++) {
		if (!pmm_test_bit(i)) {
			consecutive++;
			if (consecutive == pages) {
				return i - pages + 1;
			}
		} else {
			consecutive = 0;
		}
	}
	return 0xFFFFFFFF;
}

void pmm_init(multiboot_info_t *mb_info, uint32_t kernel_end) {
	if (!(mb_info->flags & (1 << 6))) {
		printf("PMM: Memory map not provided!\n");
		while (1) { hlt(); }
	}

	uint32_t bitmap_addr = PAGE_ALIGN(kernel_end);
	memory_base = 0xFFFFFFFF;
	uint64_t total_memory = 0;

	multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)mb_info->mmap_addr;
	uint32_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;

	while ((uint32_t)mmap < mmap_end) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			uint32_t base = mmap->base_addr_low;
			uint32_t length = mmap->length_low;

			if (base < bitmap_addr) {
				if (base + length <= bitmap_addr) {
					goto next_entry;
				}
				length -= (bitmap_addr - base);
				base = bitmap_addr;
			}

			if (base < memory_base) {
				memory_base = base;
			}
			total_memory += length;
		}
next_entry:
		mmap = (multiboot_mmap_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
	}

	total_pages = total_memory / PAGE_SIZE;
	bitmap_size = (total_pages + 7) / 8; // Округляем вверх до байта
	free_pages = total_pages;

	pmm_bitmap = (uint8_t *)bitmap_addr;
	memset(pmm_bitmap, 0, bitmap_size);

	uint32_t kernel_pages = (bitmap_addr - memory_base) / PAGE_SIZE + (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
	for (uint32_t i = 0; i < kernel_pages; i++) {
		pmm_set_bit(i);
		free_pages--;
	}

	printf("PMM: Initialized with %d total pages, %d free, bitmap at 0x%x, size %d bytes\n",
			total_pages, free_pages, bitmap_addr, bitmap_size);
}

void *pmm_alloc(uint32_t pages) {
	if (pages == 0 || pages > free_pages) {
		printf("PMM: Not enough pages (%d requested, %d free)\n", pages, free_pages);
		return NULL;
    }

	uint32_t start_idx = pmm_find_free_pages(pages);
	if (start_idx == 0xFFFFFFFF) {
		printf("PMM: No contiguous %d pages available\n", pages);
		return NULL;
	}

	for (uint32_t i = 0; i < pages; i++) {
		pmm_set_bit(start_idx + i);
	}
	free_pages -= pages;

	void *addr = (void *)(memory_base + start_idx * PAGE_SIZE);
	printf("PMM: Allocated %d pages at 0x%x\n", pages, (uint32_t)addr);
	return addr;
}

void pmm_free(void *addr, uint32_t pages) {
	if (addr == NULL || pages == 0) {
		printf("PMM: Invalid free request (addr=0x%x, pages=%d)\n", (uint32_t)addr, pages);
		return;
	}

	uint32_t start_idx = ((uint32_t)addr - memory_base) / PAGE_SIZE;
	if ((uint32_t)addr < memory_base || start_idx + pages > total_pages) {
		printf("PMM: Invalid address 0x%x for free\n", (uint32_t)addr);
		return;
	}

	for (uint32_t i = 0; i < pages; i++) {
		if (!pmm_test_bit(start_idx + i)) {
			printf("PMM: Page %d at 0x%x was not allocated\n", start_idx + i, (uint32_t)addr + i * PAGE_SIZE);
			return;
		}
	}

	for (uint32_t i = 0; i < pages; i++) {
		pmm_clear_bit(start_idx + i);
	}
	free_pages += pages;

	printf("PMM: Freed %d pages at 0x%x\n", pages, (uint32_t)addr);
}

uint32_t pmm_get_total_pages(void) {
	return total_pages;
}

uint32_t pmm_get_free_pages(void) {
	return free_pages;
}