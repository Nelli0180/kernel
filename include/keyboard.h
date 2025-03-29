#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <lib/stdint.h>

void keyboard_init(void);
void keyboard_interrupt_handler(void);
char keyboard_getc(void);

#endif /* KEYBOARD_H */
