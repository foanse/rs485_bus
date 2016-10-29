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
    struct device_attribute reley[2];/*********/
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
    B=(unsigned char *)dev->platform_data;
    B[0]=1;
    B[1]=0x11;
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
    B=(unsigned char *)dev->platform_data;
    B[0]=5;
    B[1]=0x03;
    B[2]=0;
    B[3]=192;
    B[4]=0;
    B[5]=1;
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
    B=(unsigned char *)dev->platform_data;
    B[0]=5;
    B[1]=0x06;
    B[2]=0;
    B[3]=192;
    B[4]=0;
	if(strcmp(attr->attr.name,"reley0")==0){
	    if(buf[0]=='0') B[5]=0x02;
	    if(buf[0]=='1') B[5]=0x01;
	}
	if(strcmp(attr->attr.name,"reley1")==0){
	    if(buf[0]=='0') B[5]=0x08;
	    if(buf[0]=='1') B[5]=0x04;
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
    unsigned char *B;
    struct tiny1 *T;
    int c;
    T=to_tiny(dev);
    B=(unsigned char *)dev->platform_data;
    B[0]=5;
    B[1]=0x06;
    B[2]=0;
    B[3]=195;
    B[4]=0;
    sscanf(buf, "%du", &c);
    if((c>255)||(c<1))
	return -1;
    B[5]=c;
    if(fas_rs485_bus(dev)>0){
	T->lasttime=jiffies;
	return count;
    }else{
	T->errors+=1;
	return -1;
    }
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
    unsigned char *B;
    int i;
    struct tiny1 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    printk("0x%x\t0x%x\t0x%x\t0x%x\n",buf[0],buf[1],buf[2],buf[3]);
    if((buf[1]==0x23)&&(buf[2]==0x01)){
	item=kmalloc(sizeof(struct tiny1),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct tiny1));
	    B=kmalloc(sizeof(unsigned char)*BUFSIZE,GFP_KERNEL);
	    memset(B, 0, sizeof(unsigned char)*BUFSIZE);
	    item->dev.platform_data=B;
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
		item->out[i].attr.mode=00666;
		item->out[i].show=show_out;
		item->out[i].store=store_out;
		device_create_file(&(item->dev),&(item->out[i]));
		item->block[i].attr.name=kmalloc(9,GFP_KERNEL);
		snprintf((char *)item->block[i].attr.name,9,"block_%02x",i);
		item->block[i].attr.mode=00644;
		item->block[i].show=show_block;
		item->block[i].store=store_block;
		device_create_file(&(item->dev),&(item->block[i]));
		}
	    item->version.attr.name="_version";
	    item->version.attr.mode=00444;
	    item->version.show=show_ver;
	    item->version.store=NULL;
	    device_create_file(&(item->dev),&(item->version));
	    item->error.attr.name="_errors";
	    item->error.attr.mode=00444;
	    item->error.show=show_err;
	    item->error.store=NULL;
	    device_create_file(&(item->dev),&(item->error));
	    item->last.attr.name="_sincetime";
	    item->last.attr.mode=00444;
	    item->last.show=show_last;
	    item->last.store=NULL;
	    device_create_file(&(item->dev),&(item->last));

	    item->reley[0].attr.name="reley0";
	    item->reley[0].attr.mode=00666;
	    item->reley[0].show=show_reley;
	    item->reley[0].store=store_reley;
	    device_create_file(&(item->dev),&(item->reley[0]));
	    item->reley[1].attr.name="reley1";
	    item->reley[1].attr.mode=00666;
	    item->reley[1].show=show_reley;
	    item->reley[1].store=store_reley;
	    device_create_file(&(item->dev),&(item->reley[1]));

	    item->id.attr.name="_id";
	    item->id.attr.mode=00644;
	    item->id.show=show_id;
	    item->id.store=store_id;
	    device_create_file(&(item->dev),&(item->id));


//    struct device_attribute reley[2];
//    struct device_attribute count_dev[2];
//    struct device_attribute mem[2];
//    struct device_attribute version;
//    struct device_attribute count;


	return 1;
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
	kfree(item->dev.platform_data);
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
    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;

}

module_init( dev_init );
module_exit( dev_exit );
