#include <x86.h>
#include <pic.h>

#define ICW1_INIT 0x11
#define ICW4_8086 0x01
#define PIC_EOI 0x20

void pic_init(void) {
	uint8_t mask1 = inb(PIC1_DATA);
	uint8_t mask2 = inb(PIC2_DATA);

	outb(PIC1_CMD, ICW1_INIT);
	io_wait();
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC1_DATA, 0x04);
	io_wait();
	outb(PIC1_DATA, ICW4_8086);
	io_wait();

	outb(PIC2_CMD, ICW1_INIT);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();
	outb(PIC2_DATA, 0x02);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);

	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);
}

void pic_mask_irq(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}
	value = inb(port) | (1 << irq);
	outb(port, value);
	io_wait();
}

void pic_unmask_irq(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}
	value = inb(port) & ~(1 << irq);
	outb(port, value);
	io_wait();
}

void pic_send_eoi(uint8_t irq) {
	if (irq >= 8) {
		outb(PIC2_CMD, PIC_EOI);
		io_wait();
	}
	outb(PIC1_CMD, PIC_EOI);
	io_wait();
}