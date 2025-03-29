# Компиляторы и флаги
CC = gcc
AS = nasm
LD = ld
CFLAGS = -ffreestanding -m32 -Wall -Wextra -nostdinc -Iinclude -O3 -fno-PIC -fno-stack-protector
LDFLAGS = -m elf_i386 -T kernel/linker.ld
ASFLAGS = -f elf32

# Пути
BUILD_DIR = build
KERNEL_DIR = kernel
LIB_DIR = lib
ISO_DIR = iso

OBJECTS = \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/kernel_asm.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/tss.o \
	$(BUILD_DIR)/interrupts.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/panic.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/timer.o \
	$(BUILD_DIR)/speaker.o \
	$(BUILD_DIR)/stdio.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/pmm.o \
	$(BUILD_DIR)/kheap.o \
	$(BUILD_DIR)/shell.o \
	$(BUILD_DIR)/task.o \
	$(BUILD_DIR)/task_asm.o \
	$(BUILD_DIR)/sync.o

# Цели
all: $(BUILD_DIR)/kernel.bin

# Создание директории build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Создание директории iso
$(ISO_DIR):
	mkdir -p $(ISO_DIR)

$(BUILD_DIR)/kernel.bin: $(OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_asm.o: $(KERNEL_DIR)/kernel.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/gdt.o: $(KERNEL_DIR)/gdt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interrupts.o: $(KERNEL_DIR)/interrupts.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pic.o: $(KERNEL_DIR)/pic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/panic.o: $(KERNEL_DIR)/panic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(KERNEL_DIR)/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/timer.o: $(KERNEL_DIR)/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/speaker.o: $(KERNEL_DIR)/speaker.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stdio.o: $(LIB_DIR)/stdio.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(LIB_DIR)/string.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pmm.o: $(KERNEL_DIR)/pmm.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kheap.o: $(KERNEL_DIR)/kheap.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell.o: $(KERNEL_DIR)/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tss.o: $(KERNEL_DIR)/tss.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/task.o: $(KERNEL_DIR)/task.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/task_asm.o: $(KERNEL_DIR)/task.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: $(KERNEL_DIR)/sync.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

# Цель для создания ISO с GRUB (Multiboot 1)
$(ISO_DIR)/killfence.iso: $(BUILD_DIR)/kernel.bin | $(ISO_DIR)
	rm -f $(ISO_DIR)/killfence.iso
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	echo "set timeout=5" > $(ISO_DIR)/boot/grub/grub.cfg
	echo "set default=0" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "menuentry 'KillFence Kernel' {" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "  multiboot /boot/kernel.bin" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "  boot" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "}" >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_DIR)/killfence.iso $(ISO_DIR)

# Цель для запуска ядра напрямую через QEMU
run: $(BUILD_DIR)/kernel.bin
	qemu-system-i386 -kernel $(BUILD_DIR)/kernel.bin -d int

# Цель для создания и запуска ISO через QEMU
iso: $(ISO_DIR)/killfence.iso
	qemu-system-i386 -cdrom $(ISO_DIR)/killfence.iso -d int

.PHONY: all clean run iso
