#include <linux/serial_reg.h>	// for macros related to serial register
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/delay.h>

#define PORT 0x3F8  /* Port Address Goes Here */

#define outb_s(port,command) outb(command,port)
#define inb_s(port) inb(port)

#define SIRQ	4
#define SNAME	"myserial"

  /* Defines Serial Ports Base Address */
  /* COM1 0x3F8                        */
  /* COM2 0x2F8			       */
  /* COM3 0x3E8			       */
  /* COM4 0x2E8			       */


int __init init_module(void)
{
    printk("init\n");
    outb(0,		PORT + UART_IER);        /* Turn off interrupts - Port1 */
    outb(UART_LCR_DLAB,	PORT + UART_LCR);  /* SET DLAB ON */

    outb(0x0C,		PORT + UART_DLL);
    outb(0x00,		PORT + UART_DLM);  /* Set Baud rate - Divisor Latch High Byte */
    outb(UART_LCR_WLEN8,PORT + UART_LCR);  /* 8 Bits, No Parity, 1 Stop Bit */
    outb(UART_FCR_TRIGGER_14|UART_FCR_ENABLE_FIFO|UART_FCR_CLEAR_RCVR|UART_FCR_CLEAR_XMIT,
			PORT + UART_FCR);
    /* FIFO Control Register */
//    outb(UART_MCR_OUT2|UART_MCR_RTS|UART_MCR_DTR,
//			PORT + UART_MCR);
    outb(UART_MCR_OUT2,	PORT + UART_MCR);
    /* Turn on DTR, RTS, and OUT2 */
    outb( inb(PORT + UART_LCR)&(~UART_LCR_DLAB),
			PORT + UART_LCR);
    /* SET DLAB OFF */
//    outb(UART_IER_RDI,	PORT + UART_IER);




//        outb(0x00, PORT + 1);    // Disable all interrupts
//        outb(0x80, PORT + 3);    // Enable DLAB (set baud rate divisor)
//        outb(0x01, PORT + 0);    // Set divisor to 1 (lo byte) 115200 baud
//        outb(0x00, PORT + 1);    //                  (hi byte)
//        outb(0x03, PORT + 3);    // 8 bits, no parity, one stop bit
//        outb(0x00, PORT + 2);    // Disable fifo
    printk("write\n");
    outb(0x01, PORT);
msleep(500);
    outb(0x02, PORT);
msleep(500);
    outb(0x03, PORT);
msleep(500);
    outb(0x04, PORT);
msleep(500);
    outb(0x05, PORT);
msleep(500);
    printk("read\n");
unsigned char T;
int c=0,count=30;
    do {
	printk("%d\n",count);
	c = inb(PORT + UART_LSR);
	if (c & 1) {
	    T = inb(PORT);
	    printk("%c(%02x)\n",T,T);
	}
msleep(500);
    count--;
    }while (count > 0);
    outb(0x06, PORT);
msleep(500);
    outb(0x07, PORT);
msleep(500);
    outb(0x08, PORT);
msleep(500);
    outb('\n', PORT);
    printk("exit\n");
    return -1;
}

