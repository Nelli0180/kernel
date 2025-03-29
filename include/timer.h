#ifndef TIMER_H
#define TIMER_H

#include <lib/stdint.h>
#include <kheap.h>

#define TIMER_FREQ 100

typedef struct {
	uint32_t expires;
	void (*callback)(void);
	uint8_t active;
	uint8_t periodic;
	uint32_t interval;
} timer_t;

typedef struct {
	volatile uint32_t ticks;
	uint32_t frequency;
	uint8_t initialized;
} system_timer_t;

extern system_timer_t system_timer;

void timer_init(void);
uint32_t timer_get_ticks(void);
void timer_create(timer_t *timer, uint32_t milliseconds, void (*callback)(void), uint8_t periodic);
void timer_update(timer_t *timer);
void timer_interrupt_handler(void);
void sleep(uint32_t milliseconds);
void usleep(uint32_t microseconds);
void sleep_callback(void);

#endif /* TIMER_H */