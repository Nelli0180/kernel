#include <keyboard.h>
#include <lib/stdio.h>
#include <x86.h>
#include <pic.h>
#include <kheap.h>
#include <panic.h>
#include <task.h>
#include <sync.h>

static const char scancode_to_char[] = {
	0,  0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char scancode_to_char_shift[] = {
	0,  0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

#define KEY_BUFFER_SIZE 256
static char *key_buffer;
static uint32_t key_buffer_head = 0;
static uint32_t key_buffer_tail = 0;
static uint32_t key_buffer_count = 0;
static mutex_t key_mutex;
static volatile uint8_t shift_pressed = 0;
static volatile uint8_t caps_lock_active = 0;

static void key_buffer_push(char c) {
	mutex_lock(&key_mutex);
	if (key_buffer_count < KEY_BUFFER_SIZE) {
		key_buffer[key_buffer_head] = c;
		key_buffer_head = (key_buffer_head + 1) % KEY_BUFFER_SIZE;
		key_buffer_count++;
	}
	mutex_unlock(&key_mutex);
}

static char key_buffer_pop(void) {
	mutex_lock(&key_mutex);
	if (key_buffer_count == 0) {
		mutex_unlock(&key_mutex);
		return 0;
	}
	char c = key_buffer[key_buffer_tail];
	key_buffer_tail = (key_buffer_tail + 1) % KEY_BUFFER_SIZE;
	key_buffer_count--;
	mutex_unlock(&key_mutex);
	return c;
}

static inline int is_letter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void keyboard_interrupt_handler(void) {
	uint8_t scancode = inb(0x60);
	uint8_t released = scancode & 0x80;
	scancode &= 0x7F;

	if (scancode >= sizeof(scancode_to_char) / sizeof(scancode_to_char[0])) {
		pic_send_eoi(1);
		return;
	}

	if (scancode == 0x2A || scancode == 0x36) {
		shift_pressed = released ? 0 : 1;
		pic_send_eoi(1);
		return;
    }

	if (scancode == 0x3A && !released) {
		caps_lock_active = !caps_lock_active;
		pic_send_eoi(1);
		return;
	}

	if (released) {
		pic_send_eoi(1);
		return;
	}

	char c = scancode_to_char[scancode];
	if (c != 0) {
		if (is_letter(c)) {
			if (caps_lock_active && !shift_pressed) {
				c = scancode_to_char_shift[scancode];
			} else if (!caps_lock_active && shift_pressed) {
				c = scancode_to_char_shift[scancode];
			} else if (caps_lock_active && shift_pressed) {
				c = scancode_to_char[scancode];
			}
		} else {
			if (shift_pressed) {
				c = scancode_to_char_shift[scancode];
			}
		}
		key_buffer_push(c);
	}

	pic_send_eoi(1);
}

void keyboard_init(void) {
	key_buffer = kmalloc(KEY_BUFFER_SIZE);
	if (!key_buffer) {
		panic_custom("Failed to allocate keyboard buffer");
	}

	key_buffer_head = 0;
	key_buffer_tail = 0;
	key_buffer_count = 0;
	shift_pressed = 0;
	caps_lock_active = 0;

	mutex_init(&key_mutex);
	pic_unmask_irq(1);
}

char keyboard_getc(void) {
	while (key_buffer_count == 0) {
		schedule();
	}
	return key_buffer_pop();
}