#ifndef SYNC_H
#define SYNC_H

#include <lib/stdint.h>
#include <list.h>
#include <task.h>

typedef struct {
	uint8_t locked;
	thread_control_block_t *owner;
	list_head_t wait_list;
} mutex_t;

typedef struct {
	uint32_t count;
	uint32_t max_count;
	list_head_t wait_list;
} semaphore_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

void semaphore_init(semaphore_t *sem, uint32_t initial_count, uint32_t max_count);
void semaphore_wait(semaphore_t *sem);
void semaphore_signal(semaphore_t *sem);

#endif /* SYNC_H */