#ifndef HEAP_H
#define HEAP_H

#include <lib/stddef.h>
#include <lib/stdint.h>
#include <list.h>

#define HEAP_MAGIC 0xDEADBEEF

typedef struct block {
	uint32_t magic;
	size_t size;
	uint8_t free;
	struct list_head list;
} block_t;

void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
int kwrite(void *ptr, const void *data, size_t size);

#endif /* HEAP_H */