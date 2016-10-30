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

#include "api.h"


MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
MODULE_DESCRIPTION( "rs485_bus" );
static int trys=4;
module_param(trys,int, S_IRUGO);

#define PORT 0x3F8
#define symbol 1
#define outb_s(port,command) outb(command,port)
#define inb_s(port) inb(port)


static unsigned short ModRTU_CRC(unsigned char *len, unsigned char *buf)
{
    unsigned short crc = 0xFFFF;
    int pos,i;
    for (pos = 0; pos < *len; pos++) {
        crc ^= (unsigned char)buf[pos];          // XOR byte into least sig. byte of crc
    for (i = 8; i != 0; i--) {    // Loop over each bit
        if ((crc & 0x0001) != 0) {      // If the LSB is set
            crc >>= 1;                    // Shift right and XOR 0xA001
            crc ^= 0xA001;
        }
    else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
    }
    return crc;
}
int rs485_talk(struct device *dev){
    unsigned char *buf,i,m,B[BUFSIZE];
    unsigned short crc;
    u32 J;
    do{
	i = inb(PORT+UART_LSR);
	m=inb(PORT);
    }while( i & 1);
    buf=(unsigned char *)(dev->platform_data);
    m=buf[0]+1;
    memcpy(&B,buf,m);
    B[0]=dev->id;
    crc=ModRTU_CRC(&m,&B);		//					printk("rs485_bus talk:\t%02x\t%02x\n",dev->id,m);
    for(i=0;i<m;i++){
	outb_s(PORT,B[i]);		//					printk(" 0x%02x",buf[i]);
    }
    outb_s(PORT,(unsigned char)(crc>>8));
    outb_s(PORT,(unsigned char)(crc));	//					printk("crc:0x%02x\t0x%02x\n",(unsigned char)(crc>>8),(unsigned char)(crc));
    memset(&B,0,BUFSIZE*sizeof(unsigned char));
    J=jiffies+symbol*200;
    m=0;
    while(time_before(jiffies,J)){
	i = inb(PORT+UART_LSR);
        if (i & 1) {
	    B[m] = inb(PORT);
	    if(m<BUFSIZE) m++;
	    J=jiffies+symbol*4;
	}
    }
    if(m<4){
	printk("rs485_bus: 0x%02X device don`t responce (%d)\n",dev->id,m);
	return -1;
    }
    if(dev->id!=B[0]){
	printk("rs485_bus: 0x%02X responce error number 0x%02x\n",dev->id,B[0]);
	return -2;
    }
    if(B[1]&0x80){
	printk("rs485_bus: 0x%02X device error comand 0x%02x\n",dev->id,B[1]);
	return -3;
    }
    m-=2;
    crc=ModRTU_CRC(&m,&B);
    if((((unsigned char)(crc>>8))!=B[m])||(((unsigned char)(crc))!=B[m+1])){
	printk("rs485_bus: 0x%02X device error crc 0x%04x\n",dev->id,crc);
	return -4;
    }
    memset(buf,0,sizeof(unsigned char)*BUFSIZE);
    for(i=2;i<m;i++)
	buf[i-2]=B[i];
    return (m-2);
}


extern int fas_rs485_bus(struct device *dev){
    unsigned char i,j;
    for (i=0;i<trys;i++){
	j=rs485_talk(dev);
	if(j>0) return j;
    }
}
EXPORT_SYMBOL( fas_rs485_bus );

static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "123\n");
}
static BUS_ATTR(version, 00444, show_bus_version, NULL);

static int dr_probe(struct device_driver *dr, void *dev){
    printk("driver probe...\n");
    return dr->probe(dev);
}
static int dv_probe(struct device *device, void *dev){
    printk("device probe...\n");
    struct device *D;
    D=(struct device *)dev;
    if(device->id==D->id)
	return 1;
    else
	return 0;
}

static ssize_t show_count(struct bus_type *bus, char *buf)
{
    int i,count;
    struct device *dev;
    unsigned char B[BUFSIZE];
    count=0;
    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
	for(i=10;i<17;i++){
	    memset(dev, 0, sizeof(struct device));
	    memset(&B,0,sizeof(unsigned char)*BUFSIZE);
	    dev->id=i;
	    dev->bus=bus;
	    dev->platform_data=&B;
	    B[0]=1;
	    B[1]=0x11;
	    if(fas_rs485_bus(dev)>0){
		memcpy(&(dev->dma_mask),&B,4);
		printk("Rx:0x%02x%02x%02x%02x\n",B[0],B[1],B[2],B[3]);
//		driver->probe(dev);
		if(bus_for_each_dev(bus,NULL,dev,dv_probe))
		    count++;
		else
		    count+=bus_for_each_drv(bus,NULL,dev,dr_probe);
	    }
	printk("count:%03d\ti:%03d\n",count,i);
	}
    }
    return snprintf(buf, PAGE_SIZE, "%03d\n",count);
}
static ssize_t store_count(struct bus_type *bus, const char *buf,size_t count)
{
    int i;
    struct device *dev;
    unsigned char B[BUFSIZE];
    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
	memset(dev, 0, sizeof(struct device));
	memset(&B,0,sizeof(unsigned char)*BUFSIZE);
	sscanf(buf, "%du", &i);
	dev->id=i;
	dev->bus=bus;
	dev->platform_data=&B;
	B[0]=1;
	B[1]=0x11;
	if(fas_rs485_bus(dev)>0){
	    memcpy(&(dev->dma_mask),&B,4);
	    printk("Rx:0x%02x%02x%02x%02x\n",B[0],B[1],B[2],B[3]);
		if(bus_for_each_dev(bus,NULL,dev,dv_probe))
		    return -1;
		if(bus_for_each_drv(bus,NULL,dev,dr_probe))
		    return count;
	    }
	}
    return -1;
}


static BUS_ATTR(count, 00644, show_count, store_count);


///////////////////////////////////////////////////////////////////The bus device.
static void rs485_bus_release(struct device *dev)
{
    printk(KERN_DEBUG "rs485_bus: release\n");
}
struct device rs485_bus = {
    .init_name   = "fas_rs485_0",
    .release  = rs485_bus_release
};


/////////////////////////////////////////////////////////////////// The bus type.
static int rs485_match(struct device *dev, struct device_driver *driver){
    return 0;
//!strncmp(dev->bus_id, driver->name, strlen(driver->name));
}
struct bus_type rs485_bus_type = {
    .name = "fas_rs485",
    .match = rs485_match,
//    .hotplug  = rs485_hotplug,
};



////////////////////////////////////////////////////////////// device
static void rs485_dev_release(struct device *dev){ 

}

int register_rs485_device(struct device *dev){
    unsigned char B=0;
    char buf[20];
    dev->bus = &rs485_bus_type;
    dev->parent = &rs485_bus;
    dev->release = rs485_dev_release;
    sprintf(buf, "%d:%d:%s",B,dev->id,dev->init_name);
    dev->init_name=buf;
    printk("rs485_bus: device registered!\n");
    return device_register(dev);
}
EXPORT_SYMBOL(register_rs485_device);
void unregister_rs485_device(struct device *dev)
{
    device_unregister(dev);
    dev=NULL;
    printk("rs485_bus: device unregistered!\n");

}
EXPORT_SYMBOL(unregister_rs485_device);

/*
 * Crude driver interface.
 */


int register_rs485_driver(struct device_driver *driver)
{
    int ret;
//    struct device *dev;
//    unsigned char buf[4];
    driver->bus = &rs485_bus_type;
    ret = driver_register(driver);

/*    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
        memset(dev, 0, sizeof(struct device));
	dev->id=255;
	buf[0]=0x03;
	buf[1]=0x23;
	buf[2]=0x01;
	buf[3]=0xFF;
	memcpy(&(dev->dma_mask),&buf,4);
        driver->probe(dev);
    }
*/    return ret;
}

void unregister_rs485_driver(struct device_driver *driver)
{
    driver_unregister(driver);
}
EXPORT_SYMBOL(register_rs485_driver);
EXPORT_SYMBOL(unregister_rs485_driver);


static void __exit bus_exit( void )
{
    device_unregister(&rs485_bus);
    bus_unregister(&rs485_bus_type);
    printk( KERN_ALERT "rs485_bus: is unloaded!\n" );
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



static int __init bus_init( void )
{
    serialport_init();
    int ret;
    ret = bus_register(&rs485_bus_type);
    if (ret)
	return ret;
    if (bus_create_file(&rs485_bus_type, &bus_attr_version))
	printk(KERN_NOTICE "rs485_bus: unable to create version attribute\n");
    if (bus_create_file(&rs485_bus_type, &bus_attr_count))
	printk(KERN_NOTICE "rs485_bus: unable to create count attribute\n");
    ret = device_register(&rs485_bus);
    if (ret)
	printk(KERN_NOTICE "rs485_bus: unable to register fas_rs485_0\n");
    printk( KERN_ALERT "rs485_bus: loaded!\n" );
    return ret;
}

module_init( bus_init );
module_exit( bus_exit );
