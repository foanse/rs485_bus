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
#include <linux/string.h>
#include "api.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
LIST_HEAD( list );
static int pins=8;
module_param(pins,int, S_IRUGO);

struct tiny1{
    unsigned char number;
    unsigned int  errors;
    u32 lasttime;
    struct list_head list;
    struct device dev;
    struct device_attribute *out;
    struct device_attribute *block;
    struct device_attribute reley[2];
    struct device_attribute count_dev[2];
    struct device_attribute mem[2];
    struct device_attribute version;/*********/
    struct device_attribute count;
    struct device_attribute id;
    struct device_attribute error;/*********/
    struct device_attribute last;/*********/
};
#define to_tiny(_dev) container_of(_dev, struct tiny1, dev);

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    unsigned char *B;
    struct tiny1 *T;
    T=to_tiny(dev);
    B=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
    dev->platform_data=&B;
    B[0]=20;
    B[1]=1;
    B[2]=0x11;
    if(0<fas_rs485_bus(dev)){
	T->lasttime=jiffies;
	return sprintf(buf,"0x%02X-0x%02X\n",B[1],B[2]);
    }else{
	T->errors+=1;
	return sprintf(buf,"error (%d)\n",T->errors);
    }
}
static ssize_t show_err(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    T=to_tiny(dev);
    return sprintf(buf,"%d\n",T->errors);
}
static ssize_t show_reley(struct device *dev, struct device_attribute *attr, char *buf){
    unsigned char *B;
    struct tiny1 *T;
    int c,i;
    T=to_tiny(dev);
    B=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
    dev->platform_data=B;
    B[0]=20;
    B[1]=5;
    B[2]=0x03;
    B[3]=0;
    B[4]=192;
    B[5]=0;
    B[6]=1;
    c=fas_rs485_bus(dev);
    if(0<c){
	T->lasttime=jiffies;
	if(strcmp(attr->attr.name,"reley0")==0)
	    i=(0x01&B[2]);
	if(strcmp(attr->attr.name,"reley1")==0)
	    i=(0x10&B[2])>>4;
	return sprintf(buf,"%d",i);
    }else{
	T->errors+=1;
	return sprintf(buf,"error (%d)\n",T->errors);
    }
}
static ssize_t store_reley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    unsigned char *B;
    struct tiny1 *T;
    int c,i;
    T=to_tiny(dev);
    B=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
    dev->platform_data=B;
    B[0]=20;
    B[1]=5;
    B[2]=0x06;
    B[3]=0;
    B[4]=192;
    B[5]=0;
	if(strcmp(attr->attr.name,"reley0")==0){
	    if(buf[0]=='0') B[6]=0x02;
	    if(buf[0]=='1') B[6]=0x01;
	}
	if(strcmp(attr->attr.name,"reley1")==0){
	    if(buf[0]=='0') B[6]=0x08;
	    if(buf[0]=='1') B[6]=0x04;
	}
    c=fas_rs485_bus(dev);
    if(0<c){
	T->lasttime=jiffies;
	return count;
    }else{
	T->errors+=1;
	return -1;
    }
}




static ssize_t show_id(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"%d\n",dev->id);
}
static ssize_t store_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
/*    unsigned char *B;
    struct tiny1 *T;
    T=to_tiny(dev);
    B=kmalloc(sizeof(unsigned char)*20,GFP_KERNEL);
    dev->platform_data=B;
    B[0]=20;
    B[1]=1;
    B[2]=0x11;
    if(0<fas_rs485_bus(dev)){
	T->lasttime=jiffies;
	return sprintf(buf,"0x%02X-0x%02X\n",B[1],B[2]);
    }else{
	T->errors+=1;
	return sprintf(buf,"error (%d)\n",T->errors);
    }
	kfree(buf);
*/
return -1;
}


static ssize_t show_last(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    u32 J;
    T=to_tiny(dev);
    J=(jiffies-T->lasttime)/HZ;
    if(J<0) J=10000;
    return sprintf(buf,"%d\n",J);
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
	    item->out=kmalloc(sizeof(struct device_attribute)*pins,GFP_KERNEL);
	    item->block=kmalloc(sizeof(struct device_attribute)*pins,GFP_KERNEL);
	    for (i=0;i<pins;i++){
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
	    item->last.show=show_last;
	    item->last.store=NULL;
	    device_create_file(&(item->dev),&(item->last));

	    item->reley[0].attr.name="reley0";
	    item->reley[0].attr.mode=S_IRUGO|S_IWUGO;
	    item->reley[0].show=show_reley;
	    item->reley[0].store=store_reley;
	    device_create_file(&(item->dev),&(item->reley[0]));
	    item->reley[1].attr.name="reley1";
	    item->reley[1].attr.mode=S_IRUGO|S_IWUGO;
	    item->reley[1].show=show_reley;
	    item->reley[1].store=store_reley;
	    device_create_file(&(item->dev),&(item->reley[1]));

	    item->id.attr.name="_id";
	    item->id.attr.mode=S_IRUGO|S_IWUGO;
	    item->id.show=show_id;
	    item->id.store=store_id;
	    device_create_file(&(item->dev),&(item->id));


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
	for (i=0;i<pins;i++){
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
