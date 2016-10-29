#include <linux/serial_reg.h>	// for macros related to serial register
#include <asm/io.h>

#define PORT1 0x3F8  /* Port Address Goes Here */

#define outb_s(port,command) outb(command,port)
#define inb_s(port) inb(port)

#define SIRQ	4
#define SNAME	"myserial"

  /* Defines Serial Ports Base Address */
  /* COM1 0x3F8                        */
  /* COM2 0x2F8			       */
  /* COM3 0x3E8			       */
  /* COM4 0x2E8			       */

#define BUFFERSIZE 100

char buffer_rx[BUFFERSIZE];

int init_main(void){

    outb_s(PORT1 + UART_IER , 0);        /* Turn off interrupts - Port1 */

 /*         PORT 1 - Communication Settings         */

    outb_s(PORT1 + UART_LCR , UART_LCR_DLAB);  /* SET DLAB ON */

    outb_s(PORT1 + UART_DLL , 0x0C);

    /* Set Baud rate - Divisor Latch Low Byte */
    /*         0x01 = 115,200 BPS */
    /*         0x02 =  57,600 BPS */
    /*         0x06 =  19,200 BPS */
    /*         0x0C =   9,600 BPS */
    /*         0x18 =   4,800 BPS */
    /*         0x30 =   2,400 BPS */

    outb_s(PORT1 + UART_DLM , 0x00);  /* Set Baud rate - Divisor Latch High Byte */
    
    outb_s(PORT1 + UART_LCR , UART_LCR_WLEN8);  /* 8 Bits, No Parity, 1 Stop Bit */
    
    outb_s(PORT1 + UART_FCR , UART_FCR_TRIGGER_14|UART_FCR_ENABLE_FIFO|UART_FCR_CLEAR_RCVR|UART_FCR_CLEAR_XMIT);  
    /* FIFO Control Register */
    
    outb_s(PORT1 + UART_MCR , UART_MCR_OUT2|UART_MCR_RTS|UART_MCR_DTR);  
    /* Turn on DTR, RTS, and OUT2 */
    
    outb_s(PORT1 + UART_LCR , inb_s(PORT1 + UART_LCR)&(~UART_LCR_DLAB));  
    /* SET DLAB OFF */
    
    outb_s(PORT1 + UART_IER , UART_IER_RDI);  
    /* Interrupt when data received */

    return 0;
}

//#ifdef _INTER
//void receiver(int irq, void* dev_id, struct pt_regs* regs){
//    outb_s(PORT1 + UART_IIR, (inb_s(PORT1 + UART_IIR)&(~UART_IIR_NO_INT)));	/* interrupts pending */
//#else
int receiver(void){
//#endif
    int c=0,count=0,i=9999999999;
    char temp[BUFFERSIZE];
    printk("\n<1>we receive:\n");
    do { c = inb(PORT1 + UART_LSR);
	if (c & 1) {
	    temp[count] = inb(PORT1);
	    printk("%c(%02x) ",temp[count],temp[count]);
	    count++;
	    if (count == BUFFERSIZE-1) {count = 0;}
	}
//	printk("<11111>\n");
	i--;
//    }while (c & 1);
    }while(i>0);
//#ifdef _INTER
    outb_s(PORT1 + UART_IIR, UART_IIR_NO_INT);	/* No interrupts pending */
//#else
    return 0;
//#endif
}

int __init init_module(void)
{
    int flag=0;
    printk("<1>\nPepp: Serial Module Initiating...\n");
    init_main();
//#ifdef _INTER
//    if((flag=request_irq(SIRQ,receiver,0,SNAME,NULL))!=0){
//	printk("<1>\nFailed to request irq\n");
//    }
//    else{
//	printk("<1>\nRequest irq ok\n");
//    }
//#else
    receiver();
//#endif
    return 0;
}
void __exit cleanup_module(void)
{
//#ifdef _INTER
//    free_irq(SIRQ,SNAME);
//#endif
    printk("<1>Pepp: Good-bye, kernel!\n");
}

