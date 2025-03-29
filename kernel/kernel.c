#include <lib/stdio.h>
#include <lib/string.h>
#include <multiboot.h>
#include <x86.h>
#include <gdt.h>
#include <tss.h>
#include <interrupts.h>
#include <pic.h>
#include <keyboard.h>
#include <speaker.h>
#include <pmm.h>
#include <kheap.h>
#include <timer.h>
#include <shell.h>
#include <task.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

tss_entry_t* kernel_tss;
static void* kernel_stack;

static void test_task1(void) {
	while (1) {
		printf("Test task 1 running\n");
		sleep(10000);
	}
}

static void test_task2(void) {
	while (1) {
		printf("Test task 2 running\n");
		sleep(10000);
    }
}

void kernel_main(multiboot_info_t *mb_info) {
	cli();
	clear_screen();
	gdt_init();
	pmm_init(mb_info, (uint32_t)&_kernel_end);
	heap_init();

	kernel_stack = pmm_alloc(4);
	if (!kernel_stack) {
		panic_custom("Failed to allocate kernel stack");
	}
	uint32_t kernel_stack_top = (uint32_t)kernel_stack + 16384;
	asm volatile ("mov %0, %%esp" : : "r"(kernel_stack_top));

	kernel_tss = tss_create(0x10, kernel_stack_top, 0, 0, 0, 0, 5);
	if (!kernel_tss) {
		pmm_free(kernel_stack, 4);
		panic_custom("Failed to initialize TSS");
    }

	pic_init();
	idt_init();
	timer_init();
	keyboard_init();
	speaker_init();

	initialize_multitasking();

	//create_kernel_task(test_task1, "test_task1");
	//create_kernel_task(test_task2, "test_task2");

	shell_init(mb_info);

	sti();

	printf("Starting kernel...\n");
	create_kernel_task(shell_run, "shell");

	while (1) { hlt(); }
}