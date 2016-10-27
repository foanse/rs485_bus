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
#include <linux/list.h>
#include "api.h"
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
LIST_HEAD( list );

struct tiny1{
    unsigned char number;
    struct list_head list;
    struct device dev;
};
#define to_tiny(_dev) container_of(_dev, struct tiny1, dev);
struct device_driver tiny2313a={
    .name="tiny2313a",
    
};

static int add_device(unsigned char *number, unsigned char *ID){
    struct tiny1 *item;
    item=kmalloc(sizeof(struct tiny1),GFP_KERNEL);
    if(item){
	memset(item, 0, sizeof(struct tiny1));
	item->number=number;
	item->dev.driver=&tiny2313a;
	item->dev.init_name="tiny1";
	list_add( &(item->list),&list );
	register_rs485_device(&(item->dev));
    }
}




static void __exit dev_exit( void )
{
    struct tiny1 *item;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct tiny1, list);
	unregister_rs485_device(&(item->dev));
	list_del(iter);
	kfree(item);
    }
    unregister_rs485_driver(&tiny2313a);
    printk( KERN_ALERT "rs485_dev: is unloaded!\n" );
}

static int __init dev_init( void )
{
    register_rs485_driver(&tiny2313a);
    int i;
    unsigned char *buf;
    for (i=0;i<8;i++)
	add_device(&i,buf);
    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;

}

module_init( dev_init );
module_exit( dev_exit );
