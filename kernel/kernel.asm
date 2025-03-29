section .multiboot
align 4

multiboot_header:
	dd 0x1BADB002
	dd 0x00000000
	dd -(0x1BADB002 + 0x00000000)

section .bss
align 16
boot_stack_bottom:
resb 1024
boot_stack_top:

section .text
global _start
extern kernel_main

_start:
	cmp eax, 0x2BADB002
	jne hang
	mov esp, boot_stack_top
	push ebx 
	call kernel_main
hang:
	hlt
	jmp hang
	
section .note.GNU-stack noalloc noexec nowrite progbits