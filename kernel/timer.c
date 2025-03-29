#include <timer.h>
#include <x86.h>
#include <pic.h>
#include <lib/stdio.h>
#include <kheap.h>
#include <list.h>
#include <panic.h>
#include <task.h>

#define PIT_CMD_PORT 0x43
#define PIT_DATA_PORT 0x40
#define PIT_FREQ 1193180

system_timer_t system_timer = {0, TIMER_FREQ, 0};

typedef struct timer_node {
	timer_t timer;
	list_head_t list;
} timer_node_t;

static LIST_HEAD(timer_list);

static void pit_set_frequency(uint32_t hz) {
	uint32_t divisor = PIT_FREQ / hz;
	outb(PIT_CMD_PORT, 0x36);
	outb(PIT_DATA_PORT, divisor & 0xFF);
	outb(PIT_DATA_PORT, (divisor >> 8) & 0xFF);
}

void timer_init(void) {
	if (system_timer.initialized) {
		printf("Timer: Already initialized\n");
		return;
	}

	system_timer.ticks = 0;
	system_timer.frequency = TIMER_FREQ;
	pit_set_frequency(system_timer.frequency);
	system_timer.initialized = 1;

	list_init(&timer_list);
	pic_unmask_irq(0);

	printf("Timer: System timer initialized at %d Hz\n", system_timer.frequency);
}

uint32_t timer_get_ticks(void) {
	if (!system_timer.initialized) {
		printf("Timer: Warning - accessing ticks before initialization\n");
		return 0;
	}
	return system_timer.ticks;
}

void timer_create(timer_t *timer, uint32_t milliseconds, void (*callback)(void), uint8_t periodic) {
	if (!system_timer.initialized) {
		printf("Timer: System timer not initialized, cannot create timer\n");
		return;
	}

	timer_node_t *node = (timer_node_t *)kmalloc(sizeof(timer_node_t));
	if (!node) {
		panic_custom("Failed to allocate memory for new timer");
	}

	uint32_t delta = (milliseconds * system_timer.frequency) / 1000;
	timer->expires = system_timer.ticks + delta;
	if (timer->expires < system_timer.ticks) {
		kfree(node);
		panic_custom("Timer overflow detected: requested delay too large");
	}
	timer->callback = callback;
	timer->active = 1;
	timer->periodic = periodic;
	timer->interval = delta;

	cli();
	node->timer = *timer;
	list_add(&node->list, &timer_list);
	sti();

	//printf("Timer: Created %s timer for %d ms at tick %d\n", 
	//       periodic ? "periodic" : "one-shot", milliseconds, timer->expires);
}

void timer_update(timer_t *timer) {
	if (timer->active && system_timer.ticks >= timer->expires) {
		timer->callback();
		if (timer->periodic) {
			timer->expires = system_timer.ticks + timer->interval;
			if (timer->expires < system_timer.ticks) {
				timer->expires = 0xFFFFFFFF;
			}
		} else {
			timer->active = 0;
		}
	}
}

void timer_interrupt_handler(void) {
    system_timer.ticks++;

    list_head_t *pos, *n;
    list_for_each_safe(pos, n, &timer_list) {
        timer_node_t *node = list_entry(pos, timer_node_t, list);
        if (node->timer.active) {
            timer_update(&node->timer);
            if (!node->timer.active) {
                cli();
                list_del(&node->list);
                kfree(node);
                sti();
            }
        }
    }

    thread_control_block_t* task = current_task_TCB;
    if (task) {
        thread_control_block_t* start = task;
        do {
            if (task->sleeping && task->sleep_timer.active) {
                timer_update(&task->sleep_timer);
                if (!task->sleep_timer.active) {
                    cli();
                    task->sleeping = 0;
                    task->state = TASK_STATE_READY;
                    sti();
                }
            }
            task = task->next;
        } while (task != start);
    }

    // Добавляем планировщик сюда
    schedule();
}

void sleep(uint32_t milliseconds) {
	if (!system_timer.initialized) {
		printf("Timer: Cannot sleep, system timer not initialized\n");
		return;
	}
	if (milliseconds == 0 || !current_task_TCB) {
		return;
	}

	timer_create(&current_task_TCB->sleep_timer, milliseconds, sleep_callback, 0);
	cli();
	current_task_TCB->sleeping = 1;
	current_task_TCB->state = TASK_STATE_BLOCKED;
	sti();

	schedule();
}

void sleep_callback(void) {
}

void usleep(uint32_t microseconds) {
	if (!system_timer.initialized) {
		printf("Timer: Cannot usleep, system timer not initialized\n");
		return;
	}
	if (microseconds == 0) {
		return;
	}

	uint32_t milliseconds = (microseconds + 999) / 1000;
	if (milliseconds == 0) milliseconds = 1;
	sleep(milliseconds);
}