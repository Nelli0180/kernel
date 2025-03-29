#ifndef TASK_H
#define TASK_H

#include <lib/stdint.h>
#include <timer.h>

#define TASK_STATE_RUNNING  0
#define TASK_STATE_READY    1
#define TASK_STATE_BLOCKED  2

typedef struct thread_control_block {
	void* esp;
	void* esp0;
	struct thread_control_block* next;
	uint8_t state;
	char name[32];
	timer_t sleep_timer;
	uint8_t sleeping;
} thread_control_block_t;

void initialize_multitasking(void);
void switch_to_task(thread_control_block_t* next_thread);
thread_control_block_t* create_kernel_task(void (*entry_point)(void), const char* name);
void schedule(void);

extern thread_control_block_t* current_task_TCB;

#endif /* TASK_H */