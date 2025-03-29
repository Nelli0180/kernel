#include <sync.h>
#include <kheap.h>
#include <lib/stdio.h>
#include <x86.h>
#include <panic.h>

static inline int test_and_set(uint8_t *lock) {
	uint8_t old;
	asm volatile("lock xchg %0, %1" : "=r"(old), "+m"(*lock) : "0"(1) : "memory");
	return old;
}

static inline void atomic_inc(uint32_t *value) {
	asm volatile("lock incl %0" : "+m"(*value));
}

static inline int atomic_dec(uint32_t *value) {
	int result;
	asm volatile("lock decl %0; mov %0, %1" : "+m"(*value), "=r"(result));
	return result;
}

void mutex_init(mutex_t *mutex) {
	if (!mutex) {
		panic_custom("Mutex init: NULL pointer");
	}
	mutex->locked = 0;
	mutex->owner = NULL;
	list_init(&mutex->wait_list);
}

void mutex_lock(mutex_t *mutex) {
	if (!mutex) {
		panic_custom("Mutex lock: NULL pointer");
	}

	while (test_and_set(&mutex->locked)) {
		if (mutex->owner != current_task_TCB) {
			current_task_TCB->state = TASK_STATE_BLOCKED;
			list_add_tail((list_head_t *)current_task_TCB, &mutex->wait_list);
			schedule();
		}
	}
	mutex->owner = current_task_TCB;
}

void mutex_unlock(mutex_t *mutex) {
	if (!mutex) {
		panic_custom("Mutex unlock: NULL pointer");
	}

	if (!mutex->locked || mutex->owner != current_task_TCB) {
		printf("Mutex unlock: Task '%s' does not own mutex 0x%x\n", 
			current_task_TCB->name, (uint32_t)mutex);
		return;
	}

	mutex->locked = 0;
	mutex->owner = NULL;

	if (!list_empty(&mutex->wait_list)) {
		thread_control_block_t *next_task = (thread_control_block_t *)mutex->wait_list.next;
		list_del((list_head_t *)next_task);
		next_task->state = TASK_STATE_READY;
	}
}

void semaphore_init(semaphore_t *sem, uint32_t initial_count, uint32_t max_count) {
	if (!sem || max_count == 0 || initial_count > max_count) {
		panic_custom("Semaphore init: Invalid parameters");
	}
	sem->count = initial_count;
	sem->max_count = max_count;
	list_init(&sem->wait_list);
}

void semaphore_wait(semaphore_t *sem) {
	if (!sem) {
		panic_custom("Semaphore wait: NULL pointer");
	}

	int count;
	do {
		count = atomic_dec(&sem->count);
		if (count < 0) {
			current_task_TCB->state = TASK_STATE_BLOCKED;
			list_add_tail((list_head_t *)current_task_TCB, &sem->wait_list);
			schedule();
		}
	} while (count < 0);
}

void semaphore_signal(semaphore_t *sem) {
	if (!sem) {
		panic_custom("Semaphore signal: NULL pointer");
	}

	if (sem->count < sem->max_count) {
		atomic_inc(&sem->count);
		if (!list_empty(&sem->wait_list)) {
			thread_control_block_t *next_task = (thread_control_block_t *)sem->wait_list.next;
			list_del((list_head_t *)next_task);
			next_task->state = TASK_STATE_READY;
		}
	}
}