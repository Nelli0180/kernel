#include <shell.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <x86.h>
#include <speaker.h>
#include <pmm.h>
#include <kheap.h>
#include <timer.h>
#include <keyboard.h>
#include <panic.h>
#include <task.h>
#include <sync.h>

static multiboot_info_t *global_mb_info;
static char *cmd_buffer;
static size_t cmd_len = 0;
static mutex_t vga_mutex;

static void print_memory_map(multiboot_info_t *mb_info) {
	if (!mb_info || !(mb_info->flags & (1 << 6))) {
		mutex_lock(&vga_mutex);
		printf("Memory map not provided or invalid mb_info!\n");
		mutex_unlock(&vga_mutex);
		return;
    }

	mutex_lock(&vga_mutex);
	printf("Memory map provided by bootloader:\n");
	printf("mmap_addr = 0x%x, mmap_length = %d bytes\n", mb_info->mmap_addr, mb_info->mmap_length);
	mutex_unlock(&vga_mutex);

	multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *)mb_info->mmap_addr;
	uint64_t total_available = 0;

	for (uint32_t i = 0; (uint32_t)mmap < (mb_info->mmap_addr + mb_info->mmap_length); i++) {
		mutex_lock(&vga_mutex);
		printf("Region %d: ", i);
		printf("Base: 0x%x%08x, ", mmap->base_addr_high, mmap->base_addr_low);
		printf("Length: 0x%x%08x, ", mmap->length_high, mmap->length_low);
        
		switch (mmap->type) {
			case MULTIBOOT_MEMORY_AVAILABLE:
				printf("Type: Available\n");
				total_available += ((uint64_t)mmap->length_high << 32) | mmap->length_low;
				break;
			case MULTIBOOT_MEMORY_RESERVED:
				printf("Type: Reserved\n");
				break;
			default:
				printf("Type: Unknown (%d)\n", mmap->type);
				break;
		}
		mutex_unlock(&vga_mutex);
		mmap = (multiboot_mmap_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
	}
	mutex_lock(&vga_mutex);
	printf("Total available memory: %d KB\n", (uint32_t)(total_available / 1024));
	mutex_unlock(&vga_mutex);
}

static void shell_execute(char *cmd) {
	char *args[3] = {0};
	int arg_count = 0;
	char *token = cmd;

	while (*cmd && arg_count < 3) {
		if (*cmd == ' ') {
			*cmd = '\0';
			if (token[0]) {
				args[arg_count++] = token;
			}
			token = cmd + 1;
		}
		cmd++;
	}
	if (token[0]) {
		args[arg_count++] = token;
	}

	if (args[0] == 0) {
		return;
	}

	mutex_lock(&vga_mutex);
	if (strcmp(args[0], "exit") == 0) {
		printf("Shutting down...\n");
		mutex_unlock(&vga_mutex);
		while (1) { hlt(); }
	}
	else if (strcmp(args[0], "beep") == 0) {
		uint32_t freq = 440;
		uint32_t duration = 200;
		if (arg_count > 1) freq = atoi(args[1]);
		if (arg_count > 2) duration = atoi(args[2]);
		mutex_unlock(&vga_mutex);
		speaker_beep(freq, duration);
	}
	else if (strcmp(args[0], "mem") == 0) {
		mutex_unlock(&vga_mutex);
		if (!global_mb_info) {
			mutex_lock(&vga_mutex);
			printf("Error: multiboot info not available!\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		print_memory_map(global_mb_info);
		mutex_lock(&vga_mutex);
		printf("PMM: Total pages: %d, Free pages: %d\n", pmm_get_total_pages(), pmm_get_free_pages());
		mutex_unlock(&vga_mutex);
	}
	else if (strcmp(args[0], "clear") == 0) {
		clear_screen();
		mutex_unlock(&vga_mutex);
	}
	else if (strcmp(args[0], "alloc") == 0) {
		if (arg_count < 2) {
			printf("Usage: alloc <bytes>\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		size_t bytes = atoi(args[1]);
		mutex_unlock(&vga_mutex);
		void *ptr = kmalloc(bytes);
		mutex_lock(&vga_mutex);
		if (ptr) {
			printf("Allocated %d bytes at 0x%x\n", bytes, (uint32_t)ptr);
		} else {
			printf("Failed to allocate %d bytes\n", bytes);
		}
		mutex_unlock(&vga_mutex);
	}
	else if (strcmp(args[0], "free") == 0) {
		if (arg_count < 2) {
			printf("Usage: free <address>\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		uint32_t addr = atox(args[1]);
		mutex_unlock(&vga_mutex);
		kfree((void *)addr);
	}
	else if (strcmp(args[0], "write") == 0) {
		if (arg_count < 3) {
			printf("Usage: write <address> <text>\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		uint32_t addr = atox(args[1]);
		size_t len = strlen(args[2]);
		mutex_unlock(&vga_mutex);
		int written = kwrite((void *)addr, args[2], len + 1);
		mutex_lock(&vga_mutex);
		if (written > 0) {
			printf("Wrote %d bytes ('%s') to 0x%x\n", written, args[2], addr);
		} else {
			printf("Failed to write to 0x%x\n", addr);
		}
		mutex_unlock(&vga_mutex);
	}
	else if (strcmp(args[0], "read") == 0) {
		if (arg_count < 3) {
			printf("Usage: read <address> <symbols count>\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		uint32_t addr = atox(args[1]);
		int count = atoi(args[2]);
		if (count <= 0 || count > 256) {
			printf("Invalid count: must be between 1 and 256\n");
			mutex_unlock(&vga_mutex);
			return;
		}
		char *data = (char *)addr;
		printf("Read %d symbols from 0x%x: '", count, addr);
		for (int i = 0; i < count && data[i] != '\0'; i++) {
			putchar(data[i]);
		}
		printf("'\n");
		mutex_unlock(&vga_mutex);
	}
	else if (strcmp(args[0], "yield") == 0) {
		printf("Yielding to next task...\n");
		mutex_unlock(&vga_mutex);
		schedule();
	}
	else if (strcmp(args[0], "help") == 0) {
		printf("Commands:\n");
		printf("  exit - Shut down the kernel\n");
		printf("  beep [freq] [duration] - Beep at freq Hz for duration ms\n");
		printf("  mem - Show memory map and PMM status\n");
		printf("  clear - Clear the screen\n");
		printf("  alloc <bytes> - Allocate specified number of bytes\n");
		printf("  free <address> - Free memory at specified address (hex)\n");
		printf("  write <address> <text> - Write text to specified address (hex)\n");
		printf("  read <address> <symbols count> - Read count symbols from address (hex)\n");
		printf("  yield - Switch to the next task\n");
		printf("  help - Show this help\n");
		mutex_unlock(&vga_mutex);
	}
	else {
		printf("Unknown command: %s\n", args[0]);
		mutex_unlock(&vga_mutex);
	}
}

void shell_init(multiboot_info_t *mb_info) {
	global_mb_info = mb_info;

	cmd_buffer = kmalloc(256);
	if (!cmd_buffer) {
		panic_custom("Failed to allocate command buffer");
	}

	mutex_init(&vga_mutex);
}

void shell_run(void) {
	mutex_lock(&vga_mutex);
	printf("Welcome to KillFence Kernel with Multitasking!\n");
	mutex_unlock(&vga_mutex);

	while (1) {
		mutex_lock(&vga_mutex);
		printf("> ");
		mutex_unlock(&vga_mutex);
		cmd_len = 0;
		while (1) {
			char c = keyboard_getc();
			
			mutex_lock(&vga_mutex);
			if (c == '\n') {
				putchar('\n');
				cmd_buffer[cmd_len] = '\0';
				mutex_unlock(&vga_mutex);
				shell_execute(cmd_buffer);
				break;
			} else if (c == '\b') {
				if (cmd_len > 0) {
					cmd_len--;
					putchar('\b');
					putchar(' ');
					putchar('\b');
				}
			} else if (cmd_len < 255) {
				cmd_buffer[cmd_len++] = c;
				putchar(c);
			}
			mutex_unlock(&vga_mutex);
		}
	}

	kfree(cmd_buffer);
}