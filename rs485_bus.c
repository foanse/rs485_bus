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
	printk("rs485_bus: %d:0x%02X device don`t response (%d)\n",bus,buf[0],m);
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

extern int rs485_infdev(struct device *dev){
    unsigned char i,j,a,bus,BUF[2],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x11;
    for (i=0;i<trys;i++){
	j=rs485_talk_rtu(bus, &BUF, 2, B)-1;
	if((j==B[2])&&(j>0)){
	    for(a=0;a<j;a++)
		B[a]=B[3+a];
	    return j;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_infdev );

extern int rs485_message_count(struct device *dev){
    unsigned char i,j,a,bus,BUF[2],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x0B;
    for (i=0;i<trys;i++){
	if(rs485_talk_rtu(bus, &BUF, 2, B)==2){
	    B[0]=B[2];
	    B[1]=B[3];
	    return 2;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_message_count );

extern int rs485_register_read(struct device *dev, unsigned short first, unsigned char count){
    unsigned char i,j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x04;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=0x00;
    BUF[5]=count;
    for (i=0;i<trys;i++){
	j=rs485_talk_rtu(bus, &BUF, 6,B)-1;
	if(j==(2*B[2])){
	    for(a=0;a<j;a++) B[a]=B[3+a];
	    return j;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_read );

extern int rs485_register_write(struct device *dev, unsigned short first, unsigned char count){
    unsigned char i,j,a,bus,BUF[BUFSIZE],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x10;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=0x00;
    BUF[5]=count;
    a=6+count;
    if(a>BUFSIZE) return -1;
    for(i=0;i<count;i++)
	BUF[6+i]=B[i];
    for (i=0;i<trys;i++){
	if(rs485_talk_rtu(bus, &BUF, a,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_write );

extern int rs485_register_write1(struct device *dev, unsigned short first){
    unsigned char i,j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x06;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=B[0];
    BUF[5]=B[1];
    for (i=0;i<trys;i++){
	if(rs485_talk_rtu(bus, &BUF, 6,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_write1 );

extern int rs485_coil_write(struct device *dev, unsigned short address, unsigned char val){
    unsigned char i,j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x05;
    BUF[2]=(unsigned char)(address>>8);
    BUF[3]=(unsigned char)address;
    BUF[4]=0x00;
    BUF[5]=0x00;
    if(val==1)BUF[4]=0xFF;
    if(val==0)BUF[5]=0XFF;
    for (i=0;i<trys;i++){
	if(rs485_talk_rtu(bus, &BUF, 6,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_coil_write );

extern int rs485_coil_read(struct device *dev, unsigned short first, unsigned short count){
    unsigned char i,j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x01;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=(unsigned char)(count>>8);
    BUF[5]=(unsigned char)count;
    for (i=0;i<trys;i++){
	j=rs485_talk_rtu(bus, &BUF, 6,B)-1;
	if(j==B[2]){
	    for(a=0;a<j;a++) B[a]=B[3+a];
	    return j;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_coil_read );




static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "0.1");
}
static BUS_ATTR(version, 00444, show_bus_version, NULL);

static int dr_probe(struct device_driver *dr, void *dev){
    return dr->probe(dev);
}
static int dv_probe(struct device *device, void *dev){
    struct device *D;
    D=(struct device *)dev;
    if(device->id==D->id)
	return 1;
    else
	return 0;
}

static ssize_t show_count(struct bus_type *bus, char *buf)
{
    int i,count,t;
    struct device *dev;
    unsigned char B[BUFSIZE];
    count=0;
    t=trys;
    trys=1;
    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
	for(i=1;i<256;i++){
	    memset(dev, 0, sizeof(struct device));
	    memset(&B,0,sizeof(unsigned char)*BUFSIZE);
	    dev->id=i;
	    dev->bus=bus;
	    dev->platform_data=&B;
	    if(rs485_infdev(dev)>0){
		memcpy(&(dev->dma_mask),&B,4);
		if(bus_for_each_dev(bus,NULL,dev,dv_probe))
		    count++;
		else
		    count+=bus_for_each_drv(bus,NULL,dev,dr_probe);
	    }
	}
    }
    trys=t;
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
	if(rs485_infdev(dev)>0){
	    memcpy(&(dev->dma_mask),&B,4);
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
    printk("rs485_bus: %s device registered!\n",buf);
    return device_register(dev);
}
EXPORT_SYMBOL(register_rs485_device);
void unregister_rs485_device(struct device *dev)
{
    unsigned char B=0;
    printk("rs485_bus: %d:%d:%s device unregistered!\n",B,dev->id,dev->init_name);
    device_unregister(dev);
    dev=NULL;
}
EXPORT_SYMBOL(unregister_rs485_device);

/*
 * Crude driver interface.
 */


int register_rs485_driver(struct device_driver *driver)
{
    int ret;
    driver->bus = &rs485_bus_type;
    ret = driver_register(driver);
    return ret;
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
