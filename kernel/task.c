#include <task.h>
#include <kheap.h>
#include <pmm.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <x86.h>
#include <tss.h>
#include <panic.h>
#include <sync.h>

thread_control_block_t* current_task_TCB = NULL;
static thread_control_block_t* task_list_head = NULL;

extern tss_entry_t* kernel_tss;

void initialize_multitasking(void) {
	thread_control_block_t* initial_task = (thread_control_block_t*)kmalloc(sizeof(thread_control_block_t));
	if (!initial_task) {
		panic_custom("Failed to allocate initial task TCB");
	}

	asm volatile("mov %%esp, %0" : "=r"(initial_task->esp));
	initial_task->esp0 = (void*)kernel_tss->esp0;
	initial_task->state = TASK_STATE_RUNNING;
	initial_task->sleeping = 0;
	strcpy(initial_task->name, "kernel_main");
	initial_task->next = initial_task;

	current_task_TCB = initial_task;
	task_list_head = initial_task;

	printf("Multitasking: Initialized with initial task '%s' (ESP=0x%x, ESP0=0x%x)\n", 
		   initial_task->name, (uint32_t)initial_task->esp, (uint32_t)initial_task->esp0);
}

thread_control_block_t* create_kernel_task(void (*entry_point)(void), const char* name) {
	thread_control_block_t* new_task = (thread_control_block_t*)kmalloc(sizeof(thread_control_block_t));
	if (!new_task) {
		panic_custom("Failed to allocate TCB for new task");
	}

	void* stack = pmm_alloc(4);
	if (!stack) {
		kfree(new_task);
		panic_custom("Failed to allocate stack for new task");
	}
	uint32_t stack_top = (uint32_t)stack + 16384;

	new_task->esp0 = (void*)stack_top;
	new_task->state = TASK_STATE_READY;
	new_task->sleeping = 0;
	strncpy(new_task->name, name, 31);
	new_task->name[31] = '\0';

	uint32_t* stack_ptr = (uint32_t*)stack_top;
	*--stack_ptr = (uint32_t)entry_point; // EIP
	*--stack_ptr = 0;
	*--stack_ptr = 0;
	*--stack_ptr = 0;
	*--stack_ptr = 0;
	new_task->esp = (void*)stack_ptr;

	cli();
	if (task_list_head) {
		new_task->next = task_list_head->next;
		task_list_head->next = new_task;
		task_list_head = new_task;
	} else {
		new_task->next = new_task;
		task_list_head = new_task;
	}
	sti();

	printf("Task: Created '%s' (ESP=0x%x, ESP0=0x%x, Entry=0x%x)\n", 
		   new_task->name, (uint32_t)new_task->esp, (uint32_t)new_task->esp0, (uint32_t)entry_point);

	return new_task;
}

void schedule(void) {
	if (!current_task_TCB || !current_task_TCB->next) {
		return;
	}

	thread_control_block_t* next_task = current_task_TCB->next;
	thread_control_block_t* start = next_task;

	do {
		if (next_task->state == TASK_STATE_READY) {
			break;
		}
		next_task = next_task->next;
	} while (next_task != start);

	if (next_task == current_task_TCB || next_task->state != TASK_STATE_READY) {
		return;
	}

	cli();
	if (current_task_TCB->state == TASK_STATE_RUNNING) {
		current_task_TCB->state = TASK_STATE_READY;
	}
	next_task->state = TASK_STATE_RUNNING;
	sti();

	switch_to_task(next_task);
}