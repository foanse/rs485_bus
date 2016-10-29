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
#include <linux/jiffies.h>
#include "api.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
LIST_HEAD( list );
struct tiny1{
    unsigned char number;
    unsigned int  errors;
    u32 lasttime;
    struct list_head list;
    struct device dev;
    struct device_attribute out[256];
    struct device_attribute block[256];
    struct device_attribute reley[2];
    struct device_attribute count_dev[2];
    struct device_attribute mem[2];
    struct device_attribute version;
    struct device_attribute count;
    struct device_attribute error;
    struct device_attribute last;
};
#define to_tiny(_dev) container_of(_dev, struct tiny1, dev);

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    unsigned char *B;
    struct tiny1 *T;
    T=to_tiny(dev);
    B=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
    dev->platform_data=B;
    B[0]=20;
    B[1]=1;
    B[2]=0x11;
    if(0<fas_rs485_bus(dev)){
	return sprintf(buf,"0x%02X-0x%02X\n",B[1],B[2]);
	T->lasttime=jiffies;
    }else{
	return sprintf(buf,"error\n");
	T->errors++;
    }
	kfree(buf);
}
static ssize_t show_err(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    T=to_tiny(dev);
    return sprintf(buf,"%d\n",T->errors);
}
static ssize_t show_last(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    T=to_tiny(dev);
    return sprintf(buf,"%d\n",T->lasttime);
}

static ssize_t show_out(struct device *dev, struct device_attribute *attr, char *buf){
return sprintf(buf,"nice try\n");
}
static ssize_t store_out(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){

return -1;
}
static ssize_t show_block(struct device *dev, struct device_attribute *attr, char *buf){
return sprintf(buf,"nice try\n");
}
static ssize_t store_block(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){

return -1;
}


static int tiny2313a_probe(struct device *dev);
struct device_driver tiny2313a={
    .name="tiny2313a",
    .probe=tiny2313a_probe,
};
static int tiny2313a_probe(struct device *dev){
    unsigned char buf[4];
    int i;
    struct tiny1 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    printk("0x%x\t0x%x\t0x%x\t0x%x\n",buf[0],buf[1],buf[2],buf[3]);
    if((buf[1]==0x23)&&(buf[2]==0x01)){
	item=kmalloc(sizeof(struct tiny1),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct tiny1));
	    item->number=dev->id;
	    item->errors=0;
	    item->dev.id=dev->id;
	    item->dev.driver=&tiny2313a;
	    item->dev.init_name="tiny1";
	    list_add( &(item->list),&list );
	    register_rs485_device(&(item->dev));
	    for (i=0;i<256;i++){
		item->out[i].attr.name=kmalloc(7,GFP_KERNEL);
		snprintf((char *)item->out[i].attr.name,7,"out_%02x",i);
		item->out[i].attr.mode=S_IRUGO | S_IWUSR;
		item->out[i].show=show_out;
		item->out[i].store=store_out;
		device_create_file(&(item->dev),&(item->out[i]));
		item->block[i].attr.name=kmalloc(9,GFP_KERNEL);
		snprintf((char *)item->block[i].attr.name,9,"block_%02x",i);
		item->block[i].attr.mode=S_IRUGO | S_IWUSR;
		item->block[i].show=show_block;
		item->block[i].store=store_block;
		device_create_file(&(item->dev),&(item->block[i]));
		}
	    item->version.attr.name="_version";
	    item->version.attr.mode=S_IRUGO;
	    item->version.show=show_ver;
	    item->version.store=NULL;
	    device_create_file(&(item->dev),&(item->version));
	    item->error.attr.name="_errors";
	    item->error.attr.mode=S_IRUGO;
	    item->error.show=show_err;
	    item->error.store=NULL;
	    device_create_file(&(item->dev),&(item->error));
	    item->last.attr.name="_sincetime";
	    item->last.attr.mode=S_IRUGO;
	    item->last.show=show_ver;
	    item->last.store=NULL;
	    device_create_file(&(item->dev),&(item->last));


//    struct device_attribute reley[2];
//    struct device_attribute count_dev[2];
//    struct device_attribute mem[2];
//    struct device_attribute version;
//    struct device_attribute count;



	}
    }
    return 0;
}
static void __exit dev_exit( void )
{
    struct tiny1 *item;
    int i;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct tiny1, list);
	for (i=0;i<256;i++){
	    device_remove_file(&(item->dev),&(item->out[i]));
	    kfree(item->out[i].attr.name);
	    device_remove_file(&(item->dev),&(item->block[i]));
	    kfree(item->block[i].attr.name);
	}
	unregister_rs485_device(&(item->dev));
	list_del(iter);
	kfree(item);
    }
    unregister_rs485_driver(&tiny2313a);
    printk( KERN_ALERT "rs485_dev: is unloaded!\n" );
}
static int __init dev_init( void )
{
/*    struct device *dev;
    unsigned char *buf;
    int i,c;
    dev=kmalloc(sizeof(struct device),GFP_KERNEL);
    if(dev){
	memset(dev, 0, sizeof(struct device));
	buf=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
	dev->platform_data=buf;
	dev->id=255;
	buf[0]=20;
	buf[1]=1;
	buf[2]=0x11;
printk("before_call\n");
	c=fas_rs485_bus(dev);
printk("after_call\n");
	if(c>0)
	    for(i=0;i<c;i++)
		printk("0x%02x\n",buf[i]);
	else
	    printk("%d\n",c);
	kfree(dev);
	kfree(buf);
    }
    return -1;
*/
    register_rs485_driver(&tiny2313a);
    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;

}

module_init( dev_init );
module_exit( dev_exit );
