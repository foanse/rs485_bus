#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/serial_reg.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/jiffies.h>


#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/semaphore.h>


static unsigned char trys,try;
#define PORT 0x3F8
#define symbol 1
#define outb_s(port,command) outb(command,port)
#define inb_s(port) inb(port)


static unsigned short ModRTU_CRC(unsigned char *len, unsigned char *buf)
{
    unsigned short crc = 0xFFFF;
    int pos,i;
    for (pos = 0; pos < *len; pos++) {
        crc ^= (unsigned char)buf[pos];
    for (i = 8; i != 0; i--) {
        if ((crc & 0x0001) != 0) {
            crc >>= 1;
            crc ^= 0xA001;
        }
    else
        crc >>= 1;
    }
    }
    return crc;
}

int rs485_talk_rtu(unsigned char bus, unsigned char *buf, unsigned char count, unsigned char *res){
    unsigned char i,m;
    unsigned short crc;
    u32 J;
    do{i = inb(PORT+UART_LSR);m=inb(PORT);}while( i & 1);
    crc=ModRTU_CRC(&count,buf);
    for(i=0;i<count;i++) outb_s(PORT,buf[i]);
    outb_s(PORT,(unsigned char)(crc>>8));
    outb_s(PORT,(unsigned char)(crc));
    J=jiffies+symbol*200;
    m=0;
    while(time_before(jiffies,J)){
	i = inb(PORT+UART_LSR);
        if (i & 1) {
	    res[m] = inb(PORT);
	    if(m<BUFSIZE) m++;
	    J=jiffies+symbol*4;
	}
    }
    if(m<4){
//	printk("rs485_bus: %d:0x%02X device don`t response (%d)\n",bus,buf[0],m);
	return -1;
    }
    if(buf[0]!=res[0]){
	printk("rs485_bus: %d:0x%02X responce error number 0x%02x\n",bus,buf[0],res[0]);
	return -2;
    }
    if(res[1]&0x80){
	printk("rs485_bus: %d:0x%02X device error comand 0x%02x\n",bus,buf[0],res[1]);
	return -3;
    }
    m-=2;
    crc=ModRTU_CRC(&m,res);
    if((((unsigned char)(crc>>8))!=res[m])||(((unsigned char)(crc))!=res[m+1])){
	printk("rs485_bus: %d:0x%02X device error crc 0x%04x\n",bus,buf[0],crc);
	return -4;
    }
    return (m-2);
}


static void __init serialport_init(void){
    outb_s(PORT + UART_IER,0);/* Turn off interrupts - Port1 */
    outb_s(PORT + UART_LCR,UART_LCR_DLAB);/* SET DLAB ON */
    outb_s(PORT + UART_DLL,0x0C);
    outb_s(PORT + UART_DLM,0x00);/* Set Baud rate - Divisor Latch High Byte */
    outb_s(PORT + UART_LCR,UART_LCR_WLEN8);/* 8 Bits, No Parity, 1 Stop Bit */
    outb_s(PORT + UART_FCR,UART_FCR_TRIGGER_14|UART_FCR_ENABLE_FIFO|UART_FCR_CLEAR_RCVR|UART_FCR_CLEAR_XMIT);/* FIFO Control Register */
    outb_s(PORT + UART_MCR,UART_MCR_OUT2); /* Turn on DTR, RTS, and OUT2 */
    outb_s(PORT + UART_LCR,inb(PORT + UART_LCR)&(~UART_LCR_DLAB));/* SET DLAB OFF */

}
