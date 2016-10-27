#include <linux/device.h>
#include <linux/kernel.h>


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
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/semaphore.h>
#include <asm/io.h>
#include "api.h"


MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
MODULE_DESCRIPTION( "rs485_bus" );

/*
extern char* fas_rs485_bus(char* bus, char* address, char* comand, char* data, char* count){
printk("rs485_bus to_bus\n");
return 0;
}
EXPORT_SYMBOL( fas_rs485_bus );
*/
static int I=0;

static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "123\n");
}
static BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);


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
    sprintf(buf, "%d:%d:%s",B,I,dev->init_name);
    dev->init_name=buf;
    printk("rs485_bus: device registered!\n");
    I++;
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
    struct device *dev;
    unsigned char buf[4];
    driver->bus = &rs485_bus_type;
    ret = driver_register(driver);

    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
        memset(dev, 0, sizeof(struct device));
	buf[0]=0x01;
	buf[1]=0x01;
	buf[2]=0xAA;
	buf[3]=0xB0;
	memcpy(&(dev->dma_mask),&buf,4);
        driver->probe(dev);
    }
    return ret;
/*    driver->version_attr.attr.name = "version";
    driver->version_attr.attr.owner = driver->module;
    driver->version_attr.attr.mode = S_IRUGO;
    driver->version_attr.show = show_version;
    driver->version_attr.store = NULL;
    return driver_create_file(&driver->driver, &driver->version_attr);
*/}

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
    ret = bus_register(&rs485_bus_type);
    if (ret)
	return ret;
    if (bus_create_file(&rs485_bus_type, &bus_attr_version))
	printk(KERN_NOTICE "rs485_bus: unable to create version attribute\n");
    ret = device_register(&rs485_bus);
    if (ret)
	printk(KERN_NOTICE "rs485_bus: unable to register fas_rs485_0\n");
    printk( KERN_ALERT "rs485_bus: loaded!\n" );
    return ret;
}

module_init( bus_init );
module_exit( bus_exit );
