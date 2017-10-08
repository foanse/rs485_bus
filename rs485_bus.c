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

#include "api.h"
#include "uart.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
MODULE_DESCRIPTION( "rs485_bus" );
//static int trys=10;
//static int try;
//module_param(trys,int, S_IRUGO);

extern int rs485_infdev(struct device *dev){
    unsigned char j,a,bus,BUF[2],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x11;
    for (try=0;try<trys;try++){
	j=rs485_talk_rtu(bus, BUF, 2, B)-1;
	if((j==B[2])&&(j>0)){
	    for(a=0;a<j;a++)
		B[a]=B[3+a];
    printk("rs485_bus: %d:%d get answer\n",B[3],B[4]);
	    return j;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_infdev );

extern int rs485_message_count(struct device *dev){
    unsigned char bus,BUF[2],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x0B;
    for (try=0;try<trys;try++){
	if(rs485_talk_rtu(bus, BUF, 2, B)==2){
	    B[0]=B[2];
	    B[1]=B[3];
	    return 2;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_message_count );

extern int rs485_register_read(struct device *dev, unsigned short first, unsigned char count){
    unsigned char j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x04;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=0x00;
    BUF[5]=count;
    for (try=0;try<trys;try++){
	j=rs485_talk_rtu(bus, BUF, 6,B)-1;
	if(j==(2*B[2])){
	    for(a=0;a<j;a++) B[a]=B[3+a];
	    return j;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_read );

extern int rs485_register_write(struct device *dev, unsigned short first, unsigned char count){
    unsigned char i,a,bus,BUF[BUFSIZE],*B;
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
    for (try=0;try<trys;try++){
	if(rs485_talk_rtu(bus, BUF, a,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_write );

extern int rs485_register_write1(struct device *dev, unsigned short first){
    unsigned char a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x06;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=B[0];
    BUF[5]=B[1];
    for (try=0;try<trys;try++){
	if(rs485_talk_rtu(bus, BUF, 6,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_register_write1 );

extern int rs485_coil_write(struct device *dev, unsigned short address, unsigned char val){
    unsigned char a,bus,BUF[6],*B;
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
    for (try=0;try<trys;try++){
	if(rs485_talk_rtu(bus, BUF, 6,B)==4){
	    for(a=0;a<4;a++) B[a]=B[2+a];
	    return 4;
	}
    }
    return -1;
}
EXPORT_SYMBOL( rs485_coil_write );

extern int rs485_coil_read(struct device *dev, unsigned short first, unsigned short count){
    unsigned char j,a,bus,BUF[6],*B;
    B=(unsigned char *)dev->platform_data;
    bus=0;
    BUF[0]=dev->id;
    BUF[1]=0x01;
    BUF[2]=(unsigned char)(first>>8);
    BUF[3]=(unsigned char)first;
    BUF[4]=(unsigned char)(count>>8);
    BUF[5]=(unsigned char)count;
    for (try=0;try<trys;try++){
	j=rs485_talk_rtu(bus, BUF, 6,B)-1;
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

static int __init bus_init( void )
{
    int ret;
    trys=10;
    serialport_init();
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
