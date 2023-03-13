#ifndef __INTERRUPT_PIC_IRQ_H__
#define __INTERRUPT_PIC_IRQ_H__

void pic_init(void);
void pic_enable(unsigned int irq);

#define IRQ_OFFSET      32

#endif // __INTERRUPT_PIC_IRQ_H__
