#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/serial_reg.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/delay.h>

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
static DEFINE_SEMAPHORE(fos);
void __iomem *gpio;
void __iomem *uart;
#define GPIO 0x3F200000
#define GPFSEL1         0x00000004
#define GPFSEL2         0x00000008
#define GPSET0          0x0000001C
#define GPCLR0          0x00000028

#define GPPUD           0x00000094
#define GPPUDCLK0       0x00000098

#define UART 0x3F215000
#define AUX_ENABLES     0x00000004
#define AUX_MU_IO_REG   0x00000040
#define AUX_MU_IER_REG  0x00000044
#define AUX_MU_IIR_REG  0x00000048
#define AUX_MU_LCR_REG  0x0000004C
#define AUX_MU_MCR_REG  0x00000050
#define AUX_MU_LSR_REG  0x00000054
#define AUX_MU_CNTL_REG 0x00000060
#define AUX_MU_BAUD_REG 0x00000068
#define SYMB udelay(208)

uint32_t GET32 (void *address ){
    uint32_t R;
    R=ioread32(address);
    mb();
    return R;
}
void PUT32 ( void *address, uint32_t value ){
    iowrite32(value,address);
    mb();
}

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
void sendchar(unsigned char A){
    unsigned char i;
    uint32_t R;
    i=5;
    while(i>0){
	R = GET32(uart+AUX_MU_LSR_REG);
        if (R & (1<<5)) {
	    PUT32(uart+AUX_MU_IO_REG,A);
	    return;
	}
	SYMB;
	i--;
    }
}

int rs485_talk_rtu(unsigned char bus, unsigned char *buf, unsigned char count, unsigned char *res){
    unsigned char i,m;
    uint32_t R;
    unsigned short crc;
    if(down_interruptible(&fos)){
	printk("rs485_bus: Semaphor is busy\n");
	return -1;
    }
    i=4;
    m=0;
    while(i>0){
	R = GET32(uart+AUX_MU_LSR_REG);
        if (R & 1) {
	    R = GET32(uart+AUX_MU_IO_REG);
	    i = 5;
	    m++;
	    if(m>=50){
		printk("rs485_bus: %d:0x%02X line is busy\n",bus,buf[0]);
		up(&fos);
		return -1;
	    }
	}
	SYMB;
	i--;
    }
    PUT32(gpio+GPSET0,(1<<23));
    crc=ModRTU_CRC(&count,buf);
    for(i=0;i<count;i++) sendchar(buf[i]);
    sendchar((unsigned char)(crc>>8));
    sendchar((unsigned char)(crc));
    do {R=GET32(uart+AUX_MU_LSR_REG);}while(!(R & (1<<6)));
    PUT32(gpio+GPCLR0,(1<<23));
    i=100;
    m=0;
//prntk("getting: ");
    while(i>0){
	R = GET32(uart+AUX_MU_LSR_REG);
        if (R & 1) {
	    res[m] = GET32(uart+AUX_MU_IO_REG);
//	printk("0x%02x ",res[m]);
	    if(m<BUFSIZE) m++;
	    i=10;
	}
	else{
	    SYMB;
	    i--;
	}
    }
//printk("\n");
    if(m<4){
	if(trys>1)
	    printk("rs485_bus: %d:0x%02X device don`t response (%d) [%d/%d]\n",bus,buf[0],m,try,trys);
	up(&fos);
	return -1;
    }
    if(buf[0]!=res[0]){
	printk("rs485_bus: %d:0x%02X responce error number 0x%02x [%d/%d]\n",bus,buf[0],res[0],try,trys);
	up(&fos);
	return -2;
    }
    if(res[1]&0x80){
	printk("rs485_bus: %d:0x%02X device error comand 0x%02x [%d/%d]\n",bus,buf[0],res[1],try,trys);
	up(&fos);
	return -3;
    }
    m-=2;
    crc=ModRTU_CRC(&m,res);
    if((((unsigned char)(crc>>8))!=res[m])||(((unsigned char)(crc))!=res[m+1])){
	printk("rs485_bus: %d:0x%02X device error crc 0x%04x [%d/%d]\n",bus,buf[0],crc,try,trys);
	up(&fos);
	return -4;
    }
    up(&fos);
    return (m-2);
}

static void __init serialport_init(void){
    uint32_t ra;
    gpio=ioremap_nocache(GPIO,640);
    ra=GET32(gpio+GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    PUT32(gpio+GPFSEL1,ra);

    ra=GET32(gpio+GPFSEL2);
    ra&=~(7<<9); //gpio23
    ra|=1<<9;    //output
    PUT32(gpio+GPFSEL2,ra);

    PUT32(gpio+GPPUD,0);
    PUT32(gpio+GPPUDCLK0,(1<<14)|(1<<15));
    PUT32(gpio+GPPUDCLK0,0);

    uart=ioremap_nocache(UART,496);
    PUT32(uart+AUX_ENABLES,1);
    PUT32(uart+AUX_MU_IER_REG,0);
    PUT32(uart+AUX_MU_LCR_REG,3);
    PUT32(uart+AUX_MU_MCR_REG,0);
    PUT32(uart+AUX_MU_IIR_REG,0x3);
    PUT32(uart+AUX_MU_BAUD_REG,3254);
    PUT32(uart+AUX_MU_CNTL_REG,3);
}