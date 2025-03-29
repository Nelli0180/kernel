#include <kheap.h>
#include <pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <sync.h>

#define BLOCK_HEADER_SIZE sizeof(block_t)
#define MIN_ALLOC_SIZE PAGE_SIZE

static LIST_HEAD(heap_list);
static mutex_t heap_mutex;

static void split_block(block_t *block, size_t size) {
	if (!block || block->size < size + BLOCK_HEADER_SIZE + 16) {
		return;
	}

	block_t *new_block = (block_t *)((uint8_t *)block + BLOCK_HEADER_SIZE + size);
	new_block->magic = HEAP_MAGIC;
	new_block->size = block->size - size - BLOCK_HEADER_SIZE;
	new_block->free = 1;
	list_add(&new_block->list, &block->list);

	block->size = size;
}

static void merge_blocks(block_t *block) {
	if (!block || !block->free) {
		return;
	}

	list_head_t *pos, *n;
	block_t *prev_block = NULL;
	block_t *next_block = NULL;

	list_for_each_safe(pos, n, &heap_list) {
		block_t *current = list_entry(pos, block_t, list);
		if (current == block) {
			if (pos->prev != &heap_list) {
				prev_block = list_entry(pos->prev, block_t, list);
			}
			if (pos->next != &heap_list) {
				next_block = list_entry(pos->next, block_t, list);
			}
			break;
		}
	}

	if (prev_block && prev_block->free && 
		(uint8_t *)prev_block + BLOCK_HEADER_SIZE + prev_block->size == (uint8_t *)block) {
		prev_block->size += BLOCK_HEADER_SIZE + block->size;
		list_del(&block->list);
		block = prev_block;
    }

	if (next_block && next_block->free && 
		(uint8_t *)block + BLOCK_HEADER_SIZE + block->size == (uint8_t *)next_block) {
		block->size += BLOCK_HEADER_SIZE + next_block->size;
		list_del(&next_block->list);
    }
}

void heap_init(void) {
	list_init(&heap_list);
	mutex_init(&heap_mutex);
	printf("Heap: Initialized (empty)\n");
}

void *kmalloc(size_t size) {
	if (size == 0) {
		return NULL;
	}

	mutex_lock(&heap_mutex);
	size = (size + 3) & ~3;

	block_t *best = NULL;
	size_t best_size = 0xFFFFFFFF;

	list_head_t *pos;
	list_for_each(pos, &heap_list) {
		block_t *current = list_entry(pos, block_t, list);
		if (current->free && current->size >= size && current->size < best_size) {
			best = current;
			best_size = current->size;
		}
	}

	if (!best) {
		size_t required_size = BLOCK_HEADER_SIZE + size;
		size_t pages = (required_size + PAGE_SIZE - 1) / PAGE_SIZE;
		void *new_pages = pmm_alloc(pages);
		if (!new_pages) {
			printf("Heap: Out of memory for %d bytes (%d pages)\n", size, pages);
			mutex_unlock(&heap_mutex);
			return NULL;
		}

		block_t *new_block = (block_t *)new_pages;
		new_block->magic = HEAP_MAGIC;
		new_block->size = pages * PAGE_SIZE - BLOCK_HEADER_SIZE;
		new_block->free = 1;
		list_add_tail(&new_block->list, &heap_list);

		best = new_block;
	}

	split_block(best, size);

	best->free = 0;
	void *ptr = (void *)((uint8_t *)best + BLOCK_HEADER_SIZE);
	printf("Heap: Allocated %d bytes at 0x%x\n", size, (uint32_t)ptr);
	mutex_unlock(&heap_mutex);
	return ptr;
}

void kfree(void *ptr) {
	if (!ptr) {
		return;
	}

	mutex_lock(&heap_mutex);
	block_t *block = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);

	list_head_t *pos;
	int found = 0;
	list_for_each(pos, &heap_list) {
		if (pos == &block->list) {
			found = 1;
			break;
		}
	}
	if (!found || block->free || block->magic != HEAP_MAGIC) {
		printf("Heap: Invalid free at 0x%x (found=%d, free=%d, magic=0x%x)\n", 
				(uint32_t)ptr, found, block->free, block->magic);
		mutex_unlock(&heap_mutex);
		return;
    }

	printf("Heap: Freeing %d bytes at 0x%x\n", block->size, (uint32_t)ptr);
	memset(ptr, 0, block->size);
	block->free = 1;

	merge_blocks(block);

	if (block->free && ((uint32_t)block % PAGE_SIZE) == 0) {
		size_t total_size = BLOCK_HEADER_SIZE + block->size;
		size_t block_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

		int can_free = 1;
		list_for_each(pos, &heap_list) {
			block_t *other = list_entry(pos, block_t, list);
			if (other != block) {
				uint32_t other_start = (uint32_t)other;
				uint32_t other_end = other_start + BLOCK_HEADER_SIZE + other->size;
				uint32_t block_start = (uint32_t)block;
				uint32_t block_end = block_start + total_size;

				if ((other_start < block_end && other_end > block_start) || 
					(block_start < other_end && block_end > other_start)) {
					can_free = 0; // Пересечение с другим блоком
					break;
				}
			}
		}

		if (can_free && list_empty(&heap_list)) {
			printf("Heap: Releasing %d pages at 0x%x to PMM (entire heap)\n", block_pages, (uint32_t)block);
			pmm_free(block, block_pages);
			list_init(&heap_list);
		} else if (can_free && &block->list == heap_list.next && block->list.next != &heap_list) {
			printf("Heap: Releasing %d pages at 0x%x to PMM (head)\n", block_pages, (uint32_t)block);
			list_del(&block->list);
			pmm_free(block, block_pages);
		} else if (can_free && &block->list == heap_list.prev && block->list.prev != &heap_list) {
			printf("Heap: Releasing %d pages at 0x%x to PMM (tail)\n", block_pages, (uint32_t)block);
			list_del(&block->list);
			pmm_free(block, block_pages);
		}
	}

	mutex_unlock(&heap_mutex);
}

int kwrite(void *ptr, const void *data, size_t size) {
	if (!ptr || !data || size == 0) {
		printf("Heap: Invalid kwrite parameters (ptr=0x%x, data=0x%x, size=%d)\n", 
				(uint32_t)ptr, (uint32_t)data, size);
		return -1;
	}

	mutex_lock(&heap_mutex);
	block_t *block = (block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    
	list_head_t *pos;
	int found = 0;
	list_for_each(pos, &heap_list) {
		if (pos == &block->list) {
			found = 1;
			break;
		}
    }
	if (!found || block->free || block->magic != HEAP_MAGIC) {
		printf("Heap: Invalid memory block at 0x%x for kwrite\n", (uint32_t)ptr);
		mutex_unlock(&heap_mutex);
		return -1;
    }

	if (size > block->size) {
		printf("Heap: Write size %d exceeds block size %d at 0x%x\n", size, block->size, (uint32_t)ptr);
		mutex_unlock(&heap_mutex);
		return -1;
	}

	memcpy(ptr, data, size);
	printf("Heap: Wrote %d bytes to 0x%x\n", size, (uint32_t)ptr);
	mutex_unlock(&heap_mutex);
	return size;
}