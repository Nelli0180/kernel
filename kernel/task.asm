section .text
global switch_to_task
extern current_task_TCB
extern kernel_tss

switch_to_task:
	push ebx
	push esi
	push edi
	push ebp

	mov edi, [current_task_TCB]
	mov [edi + 0], esp

	mov esi, [esp + 20]
	mov [current_task_TCB], esi

	mov esp, [esi + 0]
	mov eax, [esi + 4]
	mov ebx, kernel_tss
	mov [ebx + 4], eax

	pop ebp
	pop edi
	pop esi
	pop ebx

	ret
	
section .note.GNU-stack noalloc noexec nowrite progbits