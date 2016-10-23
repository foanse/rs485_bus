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

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
MODULE_DESCRIPTION( "rs485_bus" );
MODULE_SUPPORTED_DEVICE( "test" );

static void __exit bus_exit( void )
{
 printk( KERN_ALERT "rs485_bus is unloaded!\n" );
}

static int __init bus_init( void )
{
 printk( KERN_ALERT "rs485_bus loaded!\n" );
 return 0;
}

module_init( bus_init );
module_exit( bus_exit );