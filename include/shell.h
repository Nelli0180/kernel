#ifndef SHELL_H
#define SHELL_H

#include <lib/stddef.h>
#include <multiboot.h>

void shell_init(multiboot_info_t *mb_info);
void shell_run(void);

#endif /* SHELL_H */
