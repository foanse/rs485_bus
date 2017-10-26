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
#include "stob.h"

#define DMAX 3
#define PLAN 64
#define BLOCK 68
#define EER 512

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );

struct atmega8_1{
    unsigned int  errors;
    unsigned char pins;
    unsigned char wire_count;
    unsigned char wireID[DMAX][7];
    unsigned int address;
    struct timespec lasttime;
    struct list_head list;
    struct device dev;
    struct device_attribute wire[DMAX];
    struct device_attribute *out;
    struct device_attribute *block;
    struct device_attribute reley[2];
    struct device_attribute __reley[2];
    struct device_attribute onewire_search;
    struct device_attribute onewire_id_box;
    struct device_attribute onewire_id_in;
    struct device_attribute onewire_id_out;
    struct device_attribute onewire_id_door;
    struct device_attribute wire_last;
    struct device_attribute wire_set;
    struct device_attribute wire_box;
    struct device_attribute wire_in;
    struct device_attribute wire_out;
    struct device_attribute wire_door;
    struct device_attribute auto_comand;
    struct device_attribute send_comand;
    struct device_attribute tenlog;
    struct device_attribute _tenlog;
    struct device_attribute ten[3];
    struct device_attribute mem[2];
    struct device_attribute version;
    struct device_attribute count;
    struct device_attribute id;
    struct device_attribute error;
    struct device_attribute last;
    unsigned int MEM;
};
LIST_HEAD( list );
#define to_mega(_dev) container_of(_dev, struct atmega8_1, dev);
#define EER 512

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    int i,k,r=-1;
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    k=rs485_infdev(dev);
    if((k>0)&&(k<BUFSIZE)){
	sprintf(buf,"0x%02X",B[0]);
	for(i=1;i<k;i++)
	    r=sprintf(buf,"%s-0x%02X",buf,B[i]);
	T->lasttime=CURRENT_TIME_SEC;
	return r;
    }
    T->errors++;
    return -1;
}
static ssize_t show_err(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    T=to_mega(dev);
    return sprintf(buf,"%d",T->errors);
}
static ssize_t show_count(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(rs485_message_count(dev)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%06d",((B[0]<<8)|B[1]));
}

static ssize_t show_mem(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(rs485_register_read(dev,T->MEM,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%d",B[1]);
}

static ssize_t store_mem(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    sscanf(buf, "%du", &A);
    B[0]=(A>>8);
    B[1]=A;
    if(rs485_register_write1(dev,T->MEM)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_mem0(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    T=to_mega(dev);
    return sprintf(buf,"%d",T->MEM);
}

static ssize_t store_mem0(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned int A;
    T=to_mega(dev);
    sscanf(buf, "%du", &A);
    T->MEM=A;
    return count;
}


static ssize_t store_wire1(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned int A;
    unsigned short W;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    sscanf(buf,"%d",&A);
    W=(A<<4);
    B[1]=(unsigned char)W;
    B[0]=(unsigned char)(W>>8);
    if(rs485_register_write1(dev,77)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_wire1(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned char i;
    signed short W,F;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=0;
    if(attr->attr.name[5]=='b') i=72;
    if(attr->attr.name[5]=='i') i=74;
    if(attr->attr.name[5]=='o') i=75;
    if(attr->attr.name[5]=='d') i=73;
    if(attr->attr.name[5]=='l') i=76;
    if(attr->attr.name[5]=='s') i=77;
    if(i==0) return -1;
    if(rs485_register_read(dev,i,1)!=2){
	T->errors++;
	return -1;
    }
    if((B[0]==0xFF)&&(B[1]==0xFF)) return -1;
    W=(B[0]<<8)|B[1];
    F=(W&0x000F)*625;
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%03d.%04d",(W>>4),F);
}

static ssize_t show_wire(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned char i,j,K,D[DMAX];
    signed short W,F;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    for(i=0;i<DMAX;i++)D[i]=0;
    for(i=0;i<7;i++){
	K=number(attr->attr.name[5+2*i],attr->attr.name[6+2*i]);
	for(j=0;j<DMAX;j++)
	    if(K==T->wireID[j][i]) D[j]++;
    }
    for(i=0;i<DMAX;i++)
	if(D[i]==7){
	    if(rs485_register_read(dev,(196+i),1)!=2){
		T->errors++;
		return -1;
	    }
	    if((B[0]==0xFF)&&(B[1]==0xFF)) return -1;
	    W=(B[0]<<8)|B[1];
	    F=(W&0x000F)*625;
	    T->lasttime=CURRENT_TIME_SEC;
	    return sprintf(buf,"%03d.%04d",(W>>4),F);
	}
    return -1;
}

static void create_wire(struct atmega8_1 *item,unsigned char ID[8]){
    unsigned char i;
    for(i=0;i<7;i++)
	item->wireID[item->wire_count][i]=ID[i];
    item->wire[item->wire_count].attr.name=kmalloc(23,GFP_KERNEL);
    snprintf((char *)item->wire[item->wire_count].attr.name,23,"wire-%02x%02x%02x%02x%02x%02x%02x%02x",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
    item->wire[item->wire_count].attr.mode=00444;
    item->wire[item->wire_count].show=show_wire;
    item->wire[item->wire_count].store=NULL;
    device_create_file(&(item->dev),&(item->wire[item->wire_count]));
    item->wire_count++;
}
static void remove_wire(struct atmega8_1 *item){
    int i;
    for (i=0;i<item->wire_count;i++){
	device_remove_file(&(item->dev),&(item->wire[i]));
	kfree(item->wire[i].attr.name);
    }
    item->wire_count=0;
}
static ssize_t show_onewire_search(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B,i,j,ID[8];
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    remove_wire(T);
    for(i=0;i<DMAX;i++){
        if(rs485_register_read(dev,(129+i*8),8)!=16){
	    T->errors++;
	    return -1;
	}
	for(j=0;j<8;j++) ID[j]=B[1+j*2];
	if(B[1]) create_wire(T,ID);
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%d",T->wire_count);
}

static ssize_t store_send_comand(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B,i;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    sscanf(buf, "%du", &A);
    for(i=0;i<16;i++)
	if(A&(1<<i)){
	    if(rs485_coil_write(dev,8+i,1)!=4){
		T->errors++;
		return -1;
	    }
	}
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t store_auto_comand(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    sscanf(buf, "%du", &A);
    B[0]=(unsigned char)(A>>8);
    B[1]=(unsigned char)A;
    if(rs485_register_write1(dev,2)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}
static ssize_t show_auto_comand(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=0;
    if(rs485_register_read(dev,2,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    i=(B[0]<<8)|B[1];
    return sprintf(buf,"%d",i);
}


static ssize_t store_onewire_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B,i,j;
    unsigned int A[8];
    if(attr->attr.name[12]=='b') i=32;
    if(attr->attr.name[12]=='i') i=48;
    if(attr->attr.name[12]=='o') i=56;
    if(attr->attr.name[12]=='d') i=40;
    if(i==0) return -1;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    sscanf(buf,"%2x%2x%2x%2x%2x%2x%2x%2x",&A[0],&A[1],&A[2],&A[3],&A[4],&A[5],&A[6],&A[7]);
    for(j=0;j<8;j++){
	B[1]=(unsigned char)A[j];
	if(rs485_register_write1(dev,i+j)!=4){
	    T->errors++;
	    return -1;
	}
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}
static ssize_t show_onewire_id(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i=0;
    if(attr->attr.name[12]=='b') i=32;
    if(attr->attr.name[12]=='i') i=48;
    if(attr->attr.name[12]=='o') i=56;
    if(attr->attr.name[12]=='d') i=40;
    if(i==0) return -1;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(rs485_register_read(dev,i,8)!=16){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%02x%02x%02x%02x%02x%02x%02x%02x",B[1],B[3],B[5],B[7],B[9],B[11],B[13],B[15]);
}

static ssize_t show_tenlog(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i,j;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(rs485_register_read(dev,3,1)!=2){
	T->errors++;
	return -1;
    }
    j=B[0];
    i=B[1];
    if(rs485_register_write1(dev,3)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%d\t%d",i,j);
}

static ssize_t show_logtemp(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(rs485_register_read(dev,4,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return sprintf(buf,"%d\t%d",B[1],B[0]);
}

static ssize_t show_ten(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    if(attr->attr.name[4]=='a') i=9;
    if(attr->attr.name[4]=='b') i=10;
    if(attr->attr.name[4]=='c') i=11;
    if(rs485_register_read(dev,i,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    i=(B[1]<<8)|B[0];
    return sprintf(buf,"%d",i);
}

static ssize_t show_reley(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=0;
    if(rs485_register_read(dev,8,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    if(strcmp(attr->attr.name,"reley0")==0)
	i=(0x01&B[1]);
    if(strcmp(attr->attr.name,"reley1")==0)
	i=(0x02&B[1])>>1;
    return sprintf(buf,"%d",i);
}

static ssize_t store_reley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    B[0]=0x00;
    if(!strcmp(attr->attr.name,"reley0")){
	if(buf[0]=='0') B[1]=0x02;
	if(buf[0]=='1') B[1]=0x01;
    }
    if(!strcmp(attr->attr.name,"reley1")){
	if(buf[0]=='0') B[1]=0x08;
	if(buf[0]=='1') B[1]=0x04;
    }
    if(rs485_register_write1(dev,8)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_Ereley(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=0;
    if(rs485_register_read(dev,EER+4,1)!=2){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    if(!strcmp(attr->attr.name,"__reley0"))
	i=(0x08&B[1])>>3;
    if(!strcmp(attr->attr.name,"__reley1"))
	i=(0x10&B[1])>>4;
    return sprintf(buf,"%d",i);
}

static ssize_t store_Ereley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned short B,V;
    T=to_mega(dev);
    if(!strcmp(attr->attr.name,"__reley0")) B=(EER+4)*8+3;
    if(!strcmp(attr->attr.name,"__reley1")) B=(EER+4)*8+4;
    V=100;
    if(buf[0]=='0') V=0;
    if(buf[0]=='1') V=1;
    if(V==100) return -1;
    if(rs485_coil_write(dev,B,V)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_id(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"%03d",dev->id);
}
static ssize_t store_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct atmega8_1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A>255) return -1;
    B[1]=A;
    if(rs485_register_write1(dev,0)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_last(struct device *dev, struct device_attribute *attr, char *buf){
    struct atmega8_1 *T;
    struct timespec N;
    int J;
    T=to_mega(dev);
    N=CURRENT_TIME_SEC;
    J=N.tv_sec-T->lasttime.tv_sec;
    return sprintf(buf,"%d",J);
}


static ssize_t show_out(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=number(attr->attr.name[4],attr->attr.name[5])+PLAN*8;
    if(rs485_coil_read(dev,i,1)!=1){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    if(B[0]==0x01)
	return sprintf(buf,"1");
    else
	return sprintf(buf,"0");
}
static ssize_t store_out(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct atmega8_1 *T;
    T=to_mega(dev);
    i=number(attr->attr.name[4],attr->attr.name[5]);
    switch(buf[0]){
	case '0':v=0;i+=PLAN*8;break;
	case '1':v=1;i+=PLAN*8;break;
	case 'o':v=0;i+=(EER+64)*8;break;
	case 'i':v=1;i+=(EER+64)*8;break;
	default :return -1;
    }
    if(rs485_coil_write(dev,i,v)!=4){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_block(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct atmega8_1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega(dev);
    i=number(attr->attr.name[6],attr->attr.name[7])+(BLOCK*8);
    if(rs485_coil_read(dev,i,1)!=1){
	T->errors++;
	return -1;
    }
    T->lasttime=CURRENT_TIME_SEC;
    if(B[0]==0x01)
	return sprintf(buf,"1");
    else
	return sprintf(buf,"0");
}
static ssize_t store_block(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct atmega8_1 *T;
    T=to_mega(dev);
    i=number(attr->attr.name[6],attr->attr.name[7]);
    switch(buf[0]){
	case '0':v=0;i+=BLOCK*8;break;
	case '1':v=1;i+=BLOCK*8;break;
	case 'o':v=0;i+=(EER+68)*8;break;
	case 'i':v=1;i+=(EER+68)*8;break;
	default :return -1;
    }
    if(rs485_coil_write(dev,i,v)!=4){
	T->errors++;
	return -1;
    }
	T->lasttime=CURRENT_TIME_SEC;
	return count;
}

static int atmega8_1_probe(struct device *dev);
struct device_driver atmega8a={
    .name="atmega8a",
    .probe=atmega8_1_probe,
};


static void create_pins(struct atmega8_1 *item){
    unsigned char i;
    item->pins=8;
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
static void remove_pins(struct atmega8_1 *item){
    int i;
    for (i=0;i<item->pins;i++){
	device_remove_file(&(item->dev),&(item->out[i]));
	kfree(item->out[i].attr.name);
	device_remove_file(&(item->dev),&(item->block[i]));
	kfree(item->block[i].attr.name);
    }
}


static int atmega8_1_probe(struct device *dev){
    unsigned char buf[4];
    unsigned char *B;
    struct atmega8_1 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    if((buf[0]==0x08)&&(buf[1]==0x01)){
	item=kmalloc(sizeof(struct atmega8_1),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct atmega8_1));
	    B=kmalloc(sizeof(unsigned char)*BUFSIZE,GFP_KERNEL);
	    memset(B, 0, sizeof(unsigned char)*BUFSIZE);
	    item->dev.platform_data=B;
	    item->wire_count=0;
	    item->pins=0;
	    item->lasttime=CURRENT_TIME_SEC;
	    item->errors=0;
	    item->dev.id=dev->id;
	    item->dev.driver=&atmega8a;
	    item->dev.init_name="atmega8a";
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
	    item->send_comand.attr.name="send_comand";
	    item->send_comand.attr.mode=00222;
	    item->send_comand.show=NULL;
	    item->send_comand.store=store_send_comand;
	device_create_file(&(item->dev),&(item->send_comand));
	    item->auto_comand.attr.name="_auto_comand";
	    item->auto_comand.attr.mode=00600;
	    item->auto_comand.show=show_auto_comand;
	    item->auto_comand.store=store_auto_comand;
	device_create_file(&(item->dev),&(item->auto_comand));
	    item->tenlog.attr.name="tenlog";
	    item->tenlog.attr.mode=00444;
	    item->tenlog.show=show_tenlog;
	    item->tenlog.store=NULL;
	device_create_file(&(item->dev),&(item->tenlog));
	    item->_tenlog.attr.name="_tenlog";
	    item->_tenlog.attr.mode=00444;
	    item->_tenlog.show=show_logtemp;
	    item->_tenlog.store=NULL;
	device_create_file(&(item->dev),&(item->_tenlog));
	    item->ten[0].attr.name="ten_a";
	    item->ten[0].attr.mode=00444;
	    item->ten[0].show=show_ten;
	    item->ten[0].store=NULL;
	device_create_file(&(item->dev),&(item->ten[0]));
	    item->ten[1].attr.name="ten_b";
	    item->ten[1].attr.mode=00444;
	    item->ten[1].show=show_ten;
	    item->ten[1].store=NULL;
	device_create_file(&(item->dev),&(item->ten[1]));
	    item->ten[2].attr.name="ten_c";
	    item->ten[2].attr.mode=00444;
	    item->ten[2].show=show_ten;
	    item->ten[2].store=NULL;
	device_create_file(&(item->dev),&(item->ten[2]));
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
	    item->onewire_search.attr.name="onewire_search";
	    item->onewire_search.attr.mode=00400;
	    item->onewire_search.show=show_onewire_search;
	    item->onewire_search.store=NULL;
	device_create_file(&(item->dev),&(item->onewire_search));
	    item->onewire_id_box.attr.name="_onewire_id_box";
	    item->onewire_id_box.attr.mode=00600;
	    item->onewire_id_box.show=show_onewire_id;
	    item->onewire_id_box.store=store_onewire_id;
	device_create_file(&(item->dev),&(item->onewire_id_box));
	    item->onewire_id_in.attr.name="_onewire_id_in";
	    item->onewire_id_in.attr.mode=00600;
	    item->onewire_id_in.show=show_onewire_id;
	    item->onewire_id_in.store=store_onewire_id;
	device_create_file(&(item->dev),&(item->onewire_id_in));
	    item->onewire_id_out.attr.name="_onewire_id_out";
	    item->onewire_id_out.attr.mode=00600;
	    item->onewire_id_out.show=show_onewire_id;
	    item->onewire_id_out.store=store_onewire_id;
	device_create_file(&(item->dev),&(item->onewire_id_out));
	    item->onewire_id_door.attr.name="_onewire_id_door";
	    item->onewire_id_door.attr.mode=00600;
	    item->onewire_id_door.show=show_onewire_id;
	    item->onewire_id_door.store=store_onewire_id;
	device_create_file(&(item->dev),&(item->onewire_id_door));
	    item->wire_set.attr.name="wire-set";
	    item->wire_set.attr.mode=00666;
	    item->wire_set.show=show_wire1;
	    item->wire_set.store=store_wire1;
	device_create_file(&(item->dev),&(item->wire_set));
	    item->wire_last.attr.name="wire-last";
	    item->wire_last.attr.mode=00444;
	    item->wire_last.show=show_wire1;
	    item->wire_last.store=NULL;
	    device_create_file(&(item->dev),&(item->wire_last));
	    item->wire_box.attr.name="wire-box";
	    item->wire_box.attr.mode=00444;
	    item->wire_box.show=show_wire1;
	    item->wire_box.store=NULL;
	device_create_file(&(item->dev),&(item->wire_box));
	    item->wire_in.attr.name="wire-in";
	    item->wire_in.attr.mode=00444;
	    item->wire_in.show=show_wire1;
	    item->wire_in.store=NULL;
	device_create_file(&(item->dev),&(item->wire_in));
	    item->wire_out.attr.name="wire-out";
	    item->wire_out.attr.mode=00444;
	    item->wire_out.show=show_wire1;
	    item->wire_out.store=NULL;
	device_create_file(&(item->dev),&(item->wire_out));
	    item->wire_door.attr.name="wire-door";
	    item->wire_door.attr.mode=00444;
	    item->wire_door.show=show_wire1;
	    item->wire_door.store=NULL;
	device_create_file(&(item->dev),&(item->wire_door));
	create_pins(item);
	return 1;
	}
    }
    return 0;
}
static void __exit dev_exit( void )
{
    struct atmega8_1 *item;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct atmega8_1, list);
	remove_pins(item);
	remove_wire(item);
	kfree(item->dev.platform_data);
	unregister_rs485_device(&(item->dev));
	list_del(iter);
	kfree(item);
    }
    unregister_rs485_driver(&atmega8a);
    printk( KERN_ALERT "rs485_dev: is unloaded!\n" );
}
static int __init dev_init( void )
{
    register_rs485_driver(&atmega8a);
    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;
}

module_init( dev_init );
module_exit( dev_exit );
