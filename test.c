#include <linux/serial_reg.h>	// for macros related to serial register
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/types.h>


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
    /* Turn on DTR, RTS, and OUT2 */
    outb( inb(PORT + UART_LCR)&(~UART_LCR_DLAB),
			PORT + UART_LCR);
    /* SET DLAB OFF */
    outb(UART_IER_RDI,	PORT + UART_IER);




//        outb(0x00, PORT + 1);    // Disable all interrupts
//        outb(0x80, PORT + 3);    // Enable DLAB (set baud rate divisor)
//        outb(0x01, PORT + 0);    // Set divisor to 1 (lo byte) 115200 baud
//        outb(0x00, PORT + 1);    //                  (hi byte)
//        outb(0x03, PORT + 3);    // 8 bits, no parity, one stop bit
//        outb(0x00, PORT + 2);    // Disable fifo
        outb(0x01, PORT);
        outb(0x02, PORT);
        outb(0x03, PORT);
        outb(0x04, PORT);
        outb(0x05, PORT);
        outb(0x06, PORT);
        outb(0x07, PORT);
        outb(0x08, PORT);
        outb('\n', PORT);
        printk("<1> Sent a\n");
//    }
    return -1;
}

