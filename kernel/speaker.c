#include <speaker.h>
#include <x86.h>
#include <timer.h>
#include <pic.h>
#include <lib/stdio.h>

#define PIT_CHANNEL2_DATA 0x42
#define PIT_CMD_PORT 0x43
#define SPEAKER_PORT 0x61

static timer_t beep_timer;
static volatile uint8_t beeping = 0;

static void beep_stop_callback(void) {
	speaker_stop();
	beeping = 0;
}

static void set_pit_channel2_freq(uint32_t frequency) {
	if (frequency == 0) return;

	uint32_t divisor = 1193180 / frequency;
	outb(PIT_CMD_PORT, 0xB6);
	outb(PIT_CHANNEL2_DATA, divisor & 0xFF);
	outb(PIT_CHANNEL2_DATA, (divisor >> 8) & 0xFF);
}

void speaker_init(void) {
	speaker_stop();
}

void speaker_beep(uint32_t frequency, uint32_t milliseconds) {
	if (beeping) {
		printf("Speaker is already beeping!\n");
		return;
	}

	uint8_t tmp = inb(SPEAKER_PORT);
	outb(SPEAKER_PORT, tmp | 0x03);

	set_pit_channel2_freq(frequency);

	beeping = 1;
	timer_create(&beep_timer, milliseconds, beep_stop_callback, 0);
}

void speaker_stop(void) {
	uint8_t tmp = inb(SPEAKER_PORT);
	outb(SPEAKER_PORT, tmp & 0xFC);
	beeping = 0;
}