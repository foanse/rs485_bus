//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/aio.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/time.h>
#include "api.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );

struct tiny1{
    unsigned int  errors;
    unsigned char pins;
    unsigned int address;
    struct timespec lasttime;
    struct list_head list;
    struct device dev;
    struct device_attribute *out;/*********/
    struct device_attribute *block;/*********/
    struct device_attribute reley[2];	/*********/
    struct device_attribute __reley[2];	/*********/
    struct device_attribute spi[2];
    struct device_attribute __spi[2];
    struct device_attribute mem[2];	/*********/
    struct device_attribute version;	/*********/
    struct device_attribute count;	/*********/
    struct device_attribute id;		/*********/
    struct device_attribute error;	/*********/
    struct device_attribute last;	/*********/
};
LIST_HEAD( list );
#define to_tiny(_dev) container_of(_dev, struct tiny1, dev);
unsigned int MEM;

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    int i,k,r=-1;
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    k=rs485_infdev(dev);
    if((k>0)&&(k<BUFSIZE)){
	sprintf(buf,"0x%02X",B[0]);
	for(i=1;i<k;i++)
	    r=sprintf(buf,"%s-0x%02X",buf,B[i]);
	T->lasttime=CURRENT_TIME_SEC;
	return r;
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}
static ssize_t show_err(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    T=to_tiny(dev);
    return sprintf(buf,"%d",T->errors);
}
static ssize_t show_count(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    if(rs485_message_count(dev)==2){
	i=(B[0]<<8)|B[1];
	return sprintf(buf,"%06d",i);
	T->lasttime=CURRENT_TIME_SEC;
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}

static ssize_t show_mem(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    if(rs485_register_read(dev,MEM,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",B[1]);
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}

static ssize_t store_mem(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    B[1]=A;
    if(rs485_register_write1(dev,MEM)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_mem0(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"%d",MEM);
}

static ssize_t store_mem0(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    sscanf(buf, "%du", &MEM);
    return count;
}



static ssize_t show_reley(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    i=0;
    if(rs485_register_read(dev,192,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	if(strcmp(attr->attr.name,"reley0")==0)
	    i=(0x01&B[1]);
	if(strcmp(attr->attr.name,"reley1")==0)
	    i=(0x10&B[1])>>4;
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}

static ssize_t store_reley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    B[0]=0x00;
    if(strcmp(attr->attr.name,"reley0")==0){
	if(buf[0]=='0') B[1]=0x02;
	if(buf[0]=='1') B[1]=0x01;
    }
    if(strcmp(attr->attr.name,"reley1")==0){
	if(buf[0]=='0') B[1]=0x08;
	if(buf[0]=='1') B[1]=0x04;
    }
    if(rs485_register_write1(dev,192)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_Ereley(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    i=0;
    if(rs485_register_read(dev,68,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	if(strcmp(attr->attr.name,"__reley0")==0)
	    i=(0x10&B[1])>>4;
	if(strcmp(attr->attr.name,"__reley1")==0)
	    i=(0x20&B[1])>>5;
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}

static ssize_t store_Ereley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned short B,V;
    T=to_tiny(dev);
    if(strcmp(attr->attr.name,"__reley0")==0) B=68*8+4;
    if(strcmp(attr->attr.name,"__reley1")==0) B=68*8+5;
    V=100;
    if(buf[0]=='0') V=0;
    if(buf[0]=='1') V=1;
    if(V==100) return -1;
    if(rs485_coil_write(dev,B,V)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_spi(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    if(strcmp(attr->attr.name,"_spi0")==0)
	A=193;
    if(strcmp(attr->attr.name,"_spi1")==0)
	A=194;
    if(rs485_register_read(dev,A,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",B[1]&0x3F);
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }

}
static ssize_t store_spi(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A>18) A=18;
    B[1]=A;
    if(strcmp(attr->attr.name,"_spi0")==0)
	A=193;
    if(strcmp(attr->attr.name,"_spi1")==0)
	A=194;
    if(rs485_register_write1(dev,A)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_Espi(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    if(strcmp(attr->attr.name,"__spi0")==0)
	A=66;
    if(strcmp(attr->attr.name,"__spi1")==0)
	A=67;
    if(rs485_register_read(dev,A,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",B[1]&0x3F);
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }

}
static ssize_t store_Espi(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A>18) A=18;
    B[1]=A;
    if(strcmp(attr->attr.name,"__spi0")==0)
	A=66;
    if(strcmp(attr->attr.name,"__spi1")==0)
	A=67;
    if(rs485_register_write1(dev,A)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}





static ssize_t show_id(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"%03d",dev->id);
}
static ssize_t store_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct tiny1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A>255) return -1;
    B[1]=A;
    if(rs485_register_write1(dev,195)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_last(struct device *dev, struct device_attribute *attr, char *buf){
    struct tiny1 *T;
    struct timespec N;
    int J;
    T=to_tiny(dev);
    N=CURRENT_TIME_SEC;
    J=N.tv_sec-T->lasttime.tv_sec;
    return sprintf(buf,"%d",J);
}

static unsigned char number(unsigned char f,unsigned char s){
    unsigned char R;
    switch (f){
	case '0':R=0x00;
		break;
	case '1':R=0x10;
		break;
	case '2':R=0x20;
		break;
	case '3':R=0x30;
		break;
	case '4':R=0x40;
		break;
	case '5':R=0x50;
		break;
	case '6':R=0x60;
		break;
	case '7':R=0x70;
		break;
	case '8':R=0x80;
		break;
	case '9':R=0x90;
		break;
	case 'a':
	case 'A':R=0xA0;
		break;
	case 'b':
	case 'B':R=0xB0;
		break;
	case 'c':
	case 'C':R=0xC0;
		break;
	case 'd':
	case 'D':R=0xD0;
		break;
	case 'e':
	case 'E':R|=0x0E;
		break;
	case 'f':
	case 'F':R=0xF0;
		break;
    }
    switch (s){
	case '0':R|=0x00;
		break;
	case '1':R|=0x01;
		break;
	case '2':R|=0x02;
		break;
	case '3':R|=0x03;
		break;
	case '4':R|=0x04;
		break;
	case '5':R|=0x05;
		break;
	case '6':R|=0x06;
		break;
	case '7':R|=0x07;
		break;
	case '8':R|=0x08;
		break;
	case '9':R|=0x09;
		break;
	case 'a':
	case 'A':R|=0x0A;
		break;
	case 'b':
	case 'B':R|=0x0B;
		break;
	case 'c':
	case 'C':R|=0x0C;
		break;
	case 'd':
	case 'D':R|=0x0D;
		break;
	case 'e':
	case 'E':R|=0x0E;
		break;
	case 'f':
	case 'F':R|=0x0F;
		break;
    }
    return R;
}


static ssize_t show_out(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    i=number(attr->attr.name[4],attr->attr.name[5]);
    if(rs485_coil_read(dev,i,1)==1){
	T->lasttime=CURRENT_TIME_SEC;
	if(B[0]==0x01)
	    return sprintf(buf,"1");
	else
	    return sprintf(buf,"0");
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}
static ssize_t store_out(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct tiny1 *T;
    T=to_tiny(dev);
    i=number(attr->attr.name[4],attr->attr.name[5]);
    switch(buf[0]){
	case '0':v=0;break;
	case '1':v=1;break;
	case 'o':v=0;i+=64*8+8*8;break;
	case 'i':v=1;i+=64*8+8*8;break;
    }
    if(rs485_coil_write(dev,i,v)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_block(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct tiny1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_tiny(dev);
    i=number(attr->attr.name[6],attr->attr.name[7])+(32*8);
    if(rs485_coil_read(dev,i,1)==1){
	T->lasttime=CURRENT_TIME_SEC;
	if(B[0]==0x01)
	    return sprintf(buf,"1");
	else
	    return sprintf(buf,"0");
    }else{
	T->errors++;
	return sprintf(buf,"error (%d)",T->errors);
    }
}
static ssize_t store_block(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct tiny1 *T;
    T=to_tiny(dev);
    i=number(attr->attr.name[6],attr->attr.name[7])+(32*8);
    switch(buf[0]){
	case '0':v=0;break;
	case '1':v=1;break;
	case 'o':v=0;i+=(64+40)*8;break;
	case 'i':v=1;i+=(64+40)*8;break;
    }
    if(rs485_coil_write(dev,i,v)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }

}


static int tiny2313a_probe(struct device *dev);
struct device_driver tiny2313a={
    .name="tiny2313a",
    .probe=tiny2313a_probe,
};

static void create_pins(struct tiny1 *item){
    unsigned char *B;
    int i;
    if(rs485_register_read(&(item->dev),193,2)!=4){
	item->errors++;
	return;
    }
    item->lasttime=CURRENT_TIME_SEC;
    B=(unsigned char *)item->dev.platform_data;
    i=(B[1]&0x3F)+(B[3]&0x3F);
    if(i>32) i=0;
    item->pins=i*8;
    item->out=kmalloc(sizeof(struct device_attribute)*(item->pins),GFP_KERNEL);
    item->block=kmalloc(sizeof(struct device_attribute)*(item->pins),GFP_KERNEL);
    for (i=0;i<item->pins;i++){
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

}
static void remove_pins(struct tiny1 *item){
    int i;
    for (i=0;i<item->pins;i++){
	device_remove_file(&(item->dev),&(item->out[i]));
	kfree(item->out[i].attr.name);
	device_remove_file(&(item->dev),&(item->block[i]));
	kfree(item->block[i].attr.name);
    }
}

static int tiny2313a_probe(struct device *dev){
    unsigned char buf[4];
    unsigned char *B;
    struct tiny1 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    if((buf[0]==0x23)&&(buf[1]==0x01)){
	item=kmalloc(sizeof(struct tiny1),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct tiny1));
	    B=kmalloc(sizeof(unsigned char)*BUFSIZE,GFP_KERNEL);
	    memset(B, 0, sizeof(unsigned char)*BUFSIZE);
	    item->dev.platform_data=B;
	    item->pins=0;
	    item->lasttime=CURRENT_TIME_SEC;
	    item->errors=0;
	    item->dev.id=dev->id;
	    item->dev.driver=&tiny2313a;
	    item->dev.init_name="tiny1";
	    list_add( &(item->list),&list );
	register_rs485_device(&(item->dev));
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
	    item->last.attr.name="_lasttime";
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
	    item->spi[0].attr.name="_spi0";
	    item->spi[0].attr.mode=00644;
	    item->spi[0].show=show_spi;
	    item->spi[0].store=store_spi;
	device_create_file(&(item->dev),&(item->spi[0]));
	    item->spi[1].attr.name="_spi1";
	    item->spi[1].attr.mode=00644;
	    item->spi[1].show=show_spi;
	    item->spi[1].store=store_spi;
	device_create_file(&(item->dev),&(item->spi[1]));
	    item->id.attr.name="_id";
	    item->id.attr.mode=00644;
	    item->id.show=show_id;
	    item->id.store=store_id;
	device_create_file(&(item->dev),&(item->id));
	    item->count.attr.name="_count";
	    item->count.attr.mode=00444;
	    item->count.show=show_count;
	    item->count.store=NULL;
	device_create_file(&(item->dev),&(item->count));


	    item->__reley[0].attr.name="__reley0";
	    item->__reley[0].attr.mode=00600;
	    item->__reley[0].show=show_Ereley;
	    item->__reley[0].store=store_Ereley;
	device_create_file(&(item->dev),&(item->__reley[0]));
	    item->__reley[1].attr.name="__reley1";
	    item->__reley[1].attr.mode=00600;
	    item->__reley[1].show=show_Ereley;
	    item->__reley[1].store=store_Ereley;
	device_create_file(&(item->dev),&(item->__reley[1]));
	    item->__spi[0].attr.name="__spi0";
	    item->__spi[0].attr.mode=00600;
	    item->__spi[0].show=show_Espi;
	    item->__spi[0].store=store_Espi;
	device_create_file(&(item->dev),&(item->__spi[0]));
	    item->__spi[1].attr.name="__spi1";
	    item->__spi[1].attr.mode=00600;
	    item->__spi[1].show=show_Espi;
	    item->__spi[1].store=store_Espi;
	device_create_file(&(item->dev),&(item->__spi[1]));
	    item->mem[0].attr.name="__mem_adr";
	    item->mem[0].attr.mode=00600;
	    item->mem[0].show=show_mem0;
	    item->mem[0].store=store_mem0;
	device_create_file(&(item->dev),&(item->mem[0]));
	    item->mem[1].attr.name="__mem_data";
	    item->mem[1].attr.mode=00600;
	    item->mem[1].show=show_mem;
	    item->mem[1].store=store_mem;
	device_create_file(&(item->dev),&(item->mem[1]));


	create_pins(item);
	return 1;
	}
    }
    return 0;
}
static void __exit dev_exit( void )
{
    struct tiny1 *item;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct tiny1, list);
	remove_pins(item);
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
