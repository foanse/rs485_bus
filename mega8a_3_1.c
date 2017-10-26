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

struct mega3{
    unsigned int  errors;
    unsigned int address;
    struct timespec lasttime;
    struct list_head list;
    struct device dev;
    struct device_attribute out[16];
    struct device_attribute block[16];
    struct device_attribute reley[2];
    struct device_attribute __reley[2];
    struct device_attribute jetter;
    struct device_attribute _jetter;
    struct device_attribute comand;
    struct device_attribute _auto;
    struct device_attribute tmpID[16];
    struct device_attribute tmpTEMP[16];
    struct device_attribute W1_ID[10];
    struct device_attribute W1_TEMP[10];
    struct device_attribute mem[2];
    struct device_attribute version;
    struct device_attribute count;
    struct device_attribute id;
    struct device_attribute error;
    struct device_attribute last;
};
LIST_HEAD( list );
#define to_mega3(_dev) container_of(_dev, struct mega3, dev);
#define PLAN 89
#define BLOCK 91
#define EER 512
unsigned int MEM;

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    int i,k,r=-1;
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    k=rs485_infdev(dev);
    if((k>0)&&(k<BUFSIZE)){
	sprintf(buf,"0x%02X",B[0]);
	for(i=1;i<k;i++)
	    r=sprintf(buf,"%s-0x%02X",buf,B[i]);
	T->lasttime=CURRENT_TIME_SEC;
	return r;
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t show_err(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    T=to_mega3(dev);
    return sprintf(buf,"%d",T->errors);
}
static ssize_t show_count(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(rs485_message_count(dev)==2){
	i=(B[0]<<8)|B[1];
	return sprintf(buf,"%06d",i);
	T->lasttime=CURRENT_TIME_SEC;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_mem(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(rs485_register_read(dev,MEM,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",B[1]);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_mem(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
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
    struct mega3 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    i=0;
    if(rs485_register_read(dev,8,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	if(strcmp(attr->attr.name,"reley1")==0)
	    i=(0x01&B[1]);
	if(strcmp(attr->attr.name,"reley0")==0)
	    i=(0x02&B[1])>>1;
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_reley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    if(strcmp(attr->attr.name,"reley0")==0){
	if(buf[0]=='0') B[1]=0x02;
	if(buf[0]=='1') B[1]=0x01;
    }
    if(strcmp(attr->attr.name,"reley1")==0){
	if(buf[0]=='0') B[1]=0x08;
	if(buf[0]=='1') B[1]=0x04;
    }
    if(rs485_register_write1(dev,8)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_Ereley(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    i=0;
    if(rs485_register_read(dev,EER+4,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	if(strcmp(attr->attr.name,"__reley0")==0)
	    i=(0x10&B[1])>>4;
	if(strcmp(attr->attr.name,"__reley1")==0)
	    i=(0x08&B[1])>>3;
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_Ereley(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned short B,V;
    T=to_mega3(dev);
    if(strcmp(attr->attr.name,"__reley0")==0) B=(EER+4)*8+4;
    if(strcmp(attr->attr.name,"__reley1")==0) B=(EER+4)*8+3;
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

static ssize_t show_id(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"%03d",dev->id);
}
static ssize_t store_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A>255) return -1;
    B[1]=A;
    if(rs485_register_write1(dev,0)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_last(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    struct timespec N;
    int J;
    T=to_mega3(dev);
    N=CURRENT_TIME_SEC;
    J=N.tv_sec-T->lasttime.tv_sec;
    return sprintf(buf,"%d",J);
}
static ssize_t show_jetter(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(rs485_register_read(dev,7,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",(signed char)B[1]);
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t store_jetter(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A*A>16129) return -1;
    B[1]=(signed char)A;
    if(rs485_register_write1(dev,7)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t show_Ejetter(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(rs485_register_read(dev,EER+7,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	i=(char)B[1];
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_Ejetter(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A*A>16129) return -1;
    B[1]=(signed char)A;
    if(rs485_register_write1(dev,EER+7)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_comand(struct device *dev, struct device_attribute *attr, char *buf){
    return sprintf(buf,"1 - search 1-wire\n2 - measure temperature\n3 - get all temperature\n4 - get heating temperature\n5 - added odometrs\n6 - check balans\n");
}

static ssize_t store_comand(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    switch(A){
	case 1:B[1]=0x01;break;
	case 2:B[1]=0x02;break;
	case 3:B[1]=0x04;break;
	case 4:B[1]=0x40;break;
	case 5:B[1]=0x20;break;
	case 6:B[1]=0x80;break;
    default:
	return -1;
    }
    if(rs485_register_write1(dev,1)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_auto(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(rs485_register_read(dev,6,2)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%02x",B[1]);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_auto(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega3 *T;
    unsigned char *B;
    unsigned int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    B[0]=0x00;
    sscanf(buf, "%02x", &i);
    B[1]=(unsigned char)i;
    if(rs485_register_write1(dev,2)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
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

static ssize_t show_temp(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    int i,j;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(attr->attr.name[0]=='W')
	j=93;
    else
	j=289;
    i=number('0',attr->attr.name[8])+j;
    if(rs485_register_read(dev,i,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	i=(B[0]<<8)|B[1];
	j=(i>>4);
	i=((i&0x0F)*10000)>>4;
	return sprintf(buf,"%03d.%04d",j,i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_temp_id(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega3 *T;
    unsigned char *B;
    unsigned short j;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    if(attr->attr.name[0]=='W')
	j=9;
    else
	j=129;
    j=number('0',attr->attr.name[6])*8+j;
    if(rs485_register_read(dev,j,8)==16){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%02x.%02x.%02x.%02x.%02x.%02x.%02x-%02x",B[1],B[3],B[5],B[7],B[9],B[11],B[13],B[15]);
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t store_temp_id(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    unsigned int D[8];
    struct mega3 *T;
    unsigned char *B,j,i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    sscanf(buf, "%02x.%02x.%02x.%02x.%02x.%02x.%02x-%02x", &D[0],&D[1],&D[2],&D[3],&D[4],&D[5],&D[6],&D[7]);
    if(attr->attr.name[0]=='W')
	j=9;
    else
	j=129;
    j=number('0',attr->attr.name[3])*8+j;
    for(i=0;i<8;i++){
	B[0]=0x00;
	B[1]=D[i];
	if(rs485_register_write1(dev,i+j)!=4){
	    T->errors++;
	    return -1;
	}
    }
    T->lasttime=CURRENT_TIME_SEC;
    return count;
}

static ssize_t show_out(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    i=number(attr->attr.name[4],attr->attr.name[5])+(PLAN*8);
    if(rs485_coil_read(dev,i,1)==1){
	T->lasttime=CURRENT_TIME_SEC;
	if(B[0]==0x01)
	    return sprintf(buf,"1");
	else
	    return sprintf(buf,"0");
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t store_out(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct mega3 *T;
    T=to_mega3(dev);
    i=number(attr->attr.name[4],attr->attr.name[5]);
    switch(buf[0]){
	case '0':v=0;i+=PLAN*8;break;
	case '1':v=1;i+=PLAN*8;break;
	case 'o':v=0;i+=(EER+64)*8;break;
	case 'i':v=1;i+=(EER+64)*8;break;
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
    struct mega3 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega3(dev);
    i=number(attr->attr.name[6],attr->attr.name[7])+(BLOCK*8);
    if(rs485_coil_read(dev,i,1)==1){
	T->lasttime=CURRENT_TIME_SEC;
	if(B[0]==0x01)
	    return sprintf(buf,"1");
	else
	    return sprintf(buf,"0");
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t store_block(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i,v;
    struct mega3 *T;
    T=to_mega3(dev);
    i=number(attr->attr.name[6],attr->attr.name[7]);
    switch(buf[0]){
	case '0':v=0;i+=BLOCK*8;break;
	case '1':v=1;i+=BLOCK*8;break;
	case 'o':v=0;i+=(EER+68)*8;break;
	case 'i':v=1;i+=(EER+68)*8;break;
    }
    if(rs485_coil_write(dev,i,v)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}

static int mega8a3_probe(struct device *dev);
struct device_driver mega8a3={
    .name="mega8a3",
    .probe=mega8a3_probe,
};

static int mega8a3_probe(struct device *dev){
    unsigned char buf[4];
    unsigned char *B,i;
    struct mega3 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    if((buf[0]==0x08)&&(buf[1]==0x03)){
	item=kmalloc(sizeof(struct mega3),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct mega3));
	    B=kmalloc(sizeof(unsigned char)*BUFSIZE,GFP_KERNEL);
	    memset(B, 0, sizeof(unsigned char)*BUFSIZE);
	    item->dev.platform_data=B;
	    item->lasttime=CURRENT_TIME_SEC;
	    item->errors=0;
	    item->dev.id=dev->id;
	    item->dev.driver=&mega8a3;
	    item->dev.init_name="mega3";
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
	    item->jetter.attr.name="jetter";
	    item->jetter.attr.mode=00600;
	    item->jetter.show=show_jetter;
	    item->jetter.store=store_jetter;
	device_create_file(&(item->dev),&(item->jetter));
	    item->_jetter.attr.name="__jetter";
	    item->_jetter.attr.mode=00600;
	    item->_jetter.show=show_Ejetter;
	    item->_jetter.store=store_Ejetter;
	device_create_file(&(item->dev),&(item->_jetter));
	    item->comand.attr.name="comand";
	    item->comand.attr.mode=00600;
	    item->comand.show=show_comand;
	    item->comand.store=store_comand;
	device_create_file(&(item->dev),&(item->comand));
	    item->_auto.attr.name="_auto";
	    item->_auto.attr.mode=00444;
	    item->_auto.show=show_auto;
	    item->_auto.store=store_auto;
	device_create_file(&(item->dev),&(item->_auto));

	    for (i=0;i<16;i++){
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
		item->tmpTEMP[i].attr.name=kmalloc(10,GFP_KERNEL);
		snprintf((char *)item->tmpTEMP[i].attr.name,10,"tmpTEMP_%x",i);
		item->tmpTEMP[i].attr.mode=00444;
		item->tmpTEMP[i].show=show_temp;
		item->tmpTEMP[i].store=NULL;
	    device_create_file(&(item->dev),&(item->tmpTEMP[i]));
		item->tmpID[i].attr.name=kmalloc(8,GFP_KERNEL);
		snprintf((char *)item->tmpID[i].attr.name,8,"tmpID_%x",i);
		item->tmpID[i].attr.mode=00444;
		item->tmpID[i].show=show_temp_id;
		item->tmpID[i].store=NULL;
	    device_create_file(&(item->dev),&(item->tmpID[i]));
		if(i<10){
		    item->W1_TEMP[i].attr.name=kmalloc(10,GFP_KERNEL);
		    snprintf((char *)item->W1_TEMP[i].attr.name,10,"W1_TEMP_%x",i);
		    item->W1_TEMP[i].attr.mode=00444;
		    item->W1_TEMP[i].show=show_temp;
		    item->W1_TEMP[i].store=NULL;
		device_create_file(&(item->dev),&(item->W1_TEMP[i]));
		    item->W1_ID[i].attr.name=kmalloc(8,GFP_KERNEL);
		    snprintf((char *)item->W1_ID[i].attr.name,8,"W1_ID_%x",i);
		    item->W1_ID[i].attr.mode=00644;
		    item->W1_ID[i].show=show_temp_id;
		    item->W1_ID[i].store=store_temp_id;
	    device_create_file(&(item->dev),&(item->W1_ID[i]));
		}
	    }
	return 1;
	}
    }
    return 0;
}
static void __exit dev_exit( void )
{
    struct mega3 *item;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct mega3, list);
//	remove_pins(item);
	kfree(item->dev.platform_data);
	unregister_rs485_device(&(item->dev));
	list_del(iter);
	kfree(item);
    }
    unregister_rs485_driver(&mega8a3);
    printk( KERN_ALERT "rs485_dev: mega8a3 is unloaded!\n" );
}
static int __init dev_init( void )
{
    register_rs485_driver(&mega8a3);
    printk( KERN_ALERT "rs485_dev: mega8a3 loaded!\n" );
    return 0;
}

module_init( dev_init );
module_exit( dev_exit );
