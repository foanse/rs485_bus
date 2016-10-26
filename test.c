//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/aio.h>
#include <asm/uaccess.h>

#include "api.h"
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
/*
static int __init test(void){
 printk("rs485 test start\n");
 char* B=0;
 char* R;
 B=0;
 R=to_bus(B,B,B,B,B);
 printk("result '%d'\n",R);
 return -1;
}

module_init(test);
*/

struct rs485_device tiny2313={
    .name="tiny2313",
//    .dev.init_name =,
//    dev->dev.driver = NULL;
//    dev->dev.dev.driver_data = dev;
};


/*static void register_dev(struct rs485_device *dev, int index)
{
    sprintf(dev->name, "tyni2313_%d", index);
    dev->dev.init_name = dev->name;
//    dev->dev.driver = NULL;
//    dev->dev.dev.driver_data = dev;
    register_rs485_device(&dev);
//    device_create_file(&dev->ldev.dev, &dev_attr_dev);
}
*/
static void __exit dev_exit( void )
{
//    device_unregister(&rs485_bus);
//    bus_unregister(&rs485_bus_type);
    unregister_rs485_device(&tiny2313);
    printk( KERN_ALERT "rs485_dev: is unloaded!\n" );
}

static int __init dev_init( void )
{
/*    int ret;
    ret = bus_register(&rs485_bus_type);
    if (ret)
        return ret;
    if (bus_create_file(&rs485_bus_type, &bus_attr_version))
        printk(KERN_NOTICE "rs485_bus: unable to create version attribute\n");
    ret = device_register(&rs485_bus);
    if (ret)
        printk(KERN_NOTICE "rs485_bus: unable to register fas_rs485_0\n");
*/
//    strncpy(tiny2313.name, "tiny2313", 9);
//    tiny2313.dev.init_name = tiny2313.name;
//    dev->dev.driver = NULL;
//    dev->dev.dev.driver_data = dev;
    register_rs485_device(&tiny2313);

    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;

}

module_init( dev_init );
module_exit( dev_exit );
