ENTRY(_start)

SECTIONS {
	. = 1M; /* Начинаем с 1 МБ, как ожидает multiboot */

	_kernel_start = .; /* Определяем начало ядра как глобальный символ */

	.text : {
		*(.multiboot)
        *(.text)
	}

	.data ALIGN(4K) : {
		*(.data)
	}

	.bss ALIGN(4K) : {
		*(.bss)
	}

	_kernel_end = .; /* Определяем конец ядра как глобальный символ */
}

OUTPUT_FORMAT(elf32-i386)