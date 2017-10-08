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

struct mega1{
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
    struct device_attribute LOG[2];
    struct device_attribute odometr[3];
    struct device_attribute balans;
    struct device_attribute minutes;
    struct device_attribute comand;
    struct device_attribute _auto;
    struct device_attribute TEMP_IN;
    struct device_attribute TEMP_OUT;
    struct device_attribute TEMP_BOX;
    struct device_attribute TEMP_SET;
    struct device_attribute TEMP_LAST;
    struct device_attribute TEMP_DOOR;
    struct device_attribute ID_IN;
    struct device_attribute ID_OUT;
    struct device_attribute ID_BOX;
    struct device_attribute ID_DOOR;
    struct device_attribute ID[16];
    struct device_attribute TEMP[16];


    struct device_attribute mem[2];
    struct device_attribute version;
    struct device_attribute count;
    struct device_attribute id;
    struct device_attribute error;
    struct device_attribute last;
};
LIST_HEAD( list );
#define to_mega1(_dev) container_of(_dev, struct mega1, dev);
#define PLAN 64
#define BLOCK 68
#define EER 512
unsigned int MEM;

static ssize_t show_ver(struct device *dev, struct device_attribute *attr, char *buf){
    int i,k,r=-1;
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    T=to_mega1(dev);
    return sprintf(buf,"%d",T->errors);
}
static ssize_t show_count(struct device *dev, struct device_attribute *attr, char *buf){
    int i;
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,MEM,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",B[1]);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_mem(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned short B,V;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    unsigned int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    struct timespec N;
    int J;
    T=to_mega1(dev);
    N=CURRENT_TIME_SEC;
    J=N.tv_sec-T->lasttime.tv_sec;
    return sprintf(buf,"%d",J);
}
static ssize_t show_jetter(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,7,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d",(signed char)B[1]);
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t store_jetter(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega1 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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

static ssize_t show_odometr(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    i=2048;
    if(attr->attr.name[8]=='a') i=9;
    if(attr->attr.name[8]=='b') i=10;
    if(attr->attr.name[8]=='c') i=11;
    if(rs485_register_read(dev,i,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	i=(B[0]<<8)|B[1];
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t show_balans(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,5,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	i=(short)((B[0]<<8)|B[1]);
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t show_minutes(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    i=0;
    if(rs485_register_read(dev,6,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	i=(B[0]<<8)|B[1];
	return sprintf(buf,"%d",i);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_minutes(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega1 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    B[0]=0x00;
    sscanf(buf, "%du", &A);
    if(A!=0) return -1;
    B[1]=0x00;
    if(rs485_register_write1(dev,6)==4){
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
    struct mega1 *T;
    unsigned char *B;
    int A;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,6,2)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%02x",B[1]);
    }else{
	T->errors++;
	return -1;
    }
}

static ssize_t store_auto(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    struct mega1 *T;
    unsigned char *B;
    unsigned int i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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

static ssize_t show_logt(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,4,1)==2){
	T->lasttime=CURRENT_TIME_SEC;
	return sprintf(buf,"%d %d",B[0],B[1]);
    }else{
	T->errors++;
	return -1;
    }
}
static ssize_t show_log(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B,R[2];
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    if(rs485_register_read(dev,3,1)==2){
	R[0]=B[0];
	R[1]=B[1];
	if(rs485_register_write1(dev,3)==4){
	    T->lasttime=CURRENT_TIME_SEC;
	    return sprintf(buf,"%d %d",R[0],R[1]);
	}
    }
    T->errors++;
    return -1;
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
    struct mega1 *T;
    unsigned char *B;
    int i,j;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    i=number('0',attr->attr.name[5])+289;
    if(attr->attr.name[5]=='B') i=72;
    if(attr->attr.name[5]=='D') i=73;
    if(attr->attr.name[5]=='I') i=74;
    if(attr->attr.name[5]=='O') i=75;
    if(attr->attr.name[5]=='L') i=76;
    if(attr->attr.name[5]=='S') i=77;
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
static ssize_t store_temp(struct device *dev, struct device_attribute *attr,const char *buf, size_t count){
    int i;
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    sscanf(buf, "%du", &i);
    if((i<0)||(i>85)) return -1;
    B[0]=(i>>4);
    B[1]=(i<<4);
    if(rs485_register_write1(dev,77)==4){
	T->lasttime=CURRENT_TIME_SEC;
	return count;
    }else{
	T->errors++;
	return -1;
    }
}


static ssize_t show_temp_id(struct device *dev, struct device_attribute *attr, char *buf){
    struct mega1 *T;
    unsigned char *B;
    unsigned short j;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    j=number('0',attr->attr.name[3])*8+129;
    if(attr->attr.name[3]=='B') j=32;
    if(attr->attr.name[3]=='D') j=40;
    if(attr->attr.name[3]=='I') j=48;
    if(attr->attr.name[3]=='O') j=56;
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
    struct mega1 *T;
    unsigned char *B,j,i;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
    sscanf(buf, "%02x.%02x.%02x.%02x.%02x.%02x.%02x-%02x", &D[0],&D[1],&D[2],&D[3],&D[4],&D[5],&D[6],&D[7]);
    j=number('0',attr->attr.name[3])*8+129;
    if(attr->attr.name[3]=='B') j=32;
    if(attr->attr.name[3]=='D') j=40;
    if(attr->attr.name[3]=='I') j=48;
    if(attr->attr.name[3]=='O') j=56;
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
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    T=to_mega1(dev);
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
    struct mega1 *T;
    unsigned char *B;
    B=(unsigned char *)dev->platform_data;
    T=to_mega1(dev);
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
    struct mega1 *T;
    T=to_mega1(dev);
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

static int mega8a1_probe(struct device *dev);
struct device_driver mega8a1={
    .name="mega8a1",
    .probe=mega8a1_probe,
};

static int mega8a1_probe(struct device *dev){
    unsigned char buf[4];
    unsigned char *B,i;
    struct mega1 *item;
    memcpy(&buf,&(dev->dma_mask),4);
    if((buf[0]==0x08)&&(buf[1]==0x01)){
	item=kmalloc(sizeof(struct mega1),GFP_KERNEL);
	if(item){
	    memset(item, 0, sizeof(struct mega1));
	    B=kmalloc(sizeof(unsigned char)*BUFSIZE,GFP_KERNEL);
	    memset(B, 0, sizeof(unsigned char)*BUFSIZE);
	    item->dev.platform_data=B;
	    item->lasttime=CURRENT_TIME_SEC;
	    item->errors=0;
	    item->dev.id=dev->id;
	    item->dev.driver=&mega8a1;
	    item->dev.init_name="mega1";
	    list_add( &(item->list),&list );
	register_rs485_device(&(item->dev));
	    item->version.attr.name="_version";
	    item->version.attr.mode=00444;
	    item->version.show=show_ver;
	    item->version.store=NULL;
	device_create_file(&(item->dev),&(item->version));//++++++++++
	    item->error.attr.name="_errors";
	    item->error.attr.mode=00444;
	    item->error.show=show_err;
	    item->error.store=NULL;
	device_create_file(&(item->dev),&(item->error));//++++++++++
	    item->last.attr.name="_lasttime";
	    item->last.attr.mode=00444;
	    item->last.show=show_last;
	    item->last.store=NULL;
	device_create_file(&(item->dev),&(item->last));//++++++++++
	    item->reley[0].attr.name="reley0";
	    item->reley[0].attr.mode=00666;
	    item->reley[0].show=show_reley;
	    item->reley[0].store=store_reley;
	device_create_file(&(item->dev),&(item->reley[0]));//++++++++++
	    item->reley[1].attr.name="reley1";
	    item->reley[1].attr.mode=00666;
	    item->reley[1].show=show_reley;
	    item->reley[1].store=store_reley;
	device_create_file(&(item->dev),&(item->reley[1]));//++++++++++
	    item->id.attr.name="_id";
	    item->id.attr.mode=00644;
	    item->id.show=show_id;
	    item->id.store=store_id;
	device_create_file(&(item->dev),&(item->id));//++++++++++
	    item->count.attr.name="_count";
	    item->count.attr.mode=00444;
	    item->count.show=show_count;
	    item->count.store=NULL;
	device_create_file(&(item->dev),&(item->count));//++++++++++
	    item->__reley[0].attr.name="__reley0";
	    item->__reley[0].attr.mode=00600;
	    item->__reley[0].show=show_Ereley;
	    item->__reley[0].store=store_Ereley;
	device_create_file(&(item->dev),&(item->__reley[0]));//++++++++++
	    item->__reley[1].attr.name="__reley1";
	    item->__reley[1].attr.mode=00600;
	    item->__reley[1].show=show_Ereley;
	    item->__reley[1].store=store_Ereley;
	device_create_file(&(item->dev),&(item->__reley[1]));//++++++++++
	    item->mem[0].attr.name="__mem_adr";
	    item->mem[0].attr.mode=00600;
	    item->mem[0].show=show_mem0;
	    item->mem[0].store=store_mem0;
	device_create_file(&(item->dev),&(item->mem[0]));//++++++++++
	    item->mem[1].attr.name="__mem_data";
	    item->mem[1].attr.mode=00600;
	    item->mem[1].show=show_mem;
	    item->mem[1].store=store_mem;
	device_create_file(&(item->dev),&(item->mem[1]));//++++++++++
	    item->jetter.attr.name="jetter";
	    item->jetter.attr.mode=00600;
	    item->jetter.show=show_jetter;
	    item->jetter.store=store_jetter;
	device_create_file(&(item->dev),&(item->jetter));//++++++++++
	    item->_jetter.attr.name="__jetter";
	    item->_jetter.attr.mode=00600;
	    item->_jetter.show=show_Ejetter;
	    item->_jetter.store=store_Ejetter;
	device_create_file(&(item->dev),&(item->_jetter));//++++++++++
	    item->odometr[0].attr.name="odometr_a";
	    item->odometr[0].attr.mode=00444;
	    item->odometr[0].show=show_odometr;
	    item->odometr[0].store=NULL;
	device_create_file(&(item->dev),&(item->odometr[0]));//++++++++++
	    item->odometr[1].attr.name="odometr_b";
	    item->odometr[1].attr.mode=00444;
	    item->odometr[1].show=show_odometr;
	    item->odometr[1].store=NULL;
	device_create_file(&(item->dev),&(item->odometr[1]));//++++++++++
	    item->odometr[2].attr.name="odometr_c";
	    item->odometr[2].attr.mode=00444;
	    item->odometr[2].show=show_odometr;
	    item->odometr[2].store=NULL;
	device_create_file(&(item->dev),&(item->odometr[2]));//++++++++++
	    item->balans.attr.name="balans";
	    item->balans.attr.mode=00444;
	    item->balans.show=show_balans;
	    item->balans.store=NULL;
	device_create_file(&(item->dev),&(item->balans));//++++++++++
	    item->minutes.attr.name="minutes";
	    item->minutes.attr.mode=00666;
	    item->minutes.show=show_minutes;
	    item->minutes.store=store_minutes;
	device_create_file(&(item->dev),&(item->minutes));//++++++++++
	    item->TEMP_IN.attr.name="TEMP_IN";
	    item->TEMP_IN.attr.mode=00444;
	    item->TEMP_IN.show=show_temp;
	    item->TEMP_IN.store=NULL;
	device_create_file(&(item->dev),&(item->TEMP_IN));//++++++++++
	    item->TEMP_OUT.attr.name="TEMP_OUT";
	    item->TEMP_OUT.attr.mode=00444;
	    item->TEMP_OUT.show=show_temp;
	    item->TEMP_OUT.store=NULL;
	device_create_file(&(item->dev),&(item->TEMP_OUT));//++++++++++
	    item->TEMP_BOX.attr.name="TEMP_BOX";
	    item->TEMP_BOX.attr.mode=00444;
	    item->TEMP_BOX.show=show_temp;
	    item->TEMP_BOX.store=NULL;
	device_create_file(&(item->dev),&(item->TEMP_BOX));//++++++++++
	    item->TEMP_SET.attr.name="TEMP_SET";
	    item->TEMP_SET.attr.mode=00666;
	    item->TEMP_SET.show=show_temp;
	    item->TEMP_SET.store=store_temp;
	device_create_file(&(item->dev),&(item->TEMP_SET));
	    item->TEMP_LAST.attr.name="TEMP_LAST";
	    item->TEMP_LAST.attr.mode=00444;
	    item->TEMP_LAST.show=show_temp;
	    item->TEMP_LAST.store=NULL;
	device_create_file(&(item->dev),&(item->TEMP_LAST));//++++++++++
	    item->TEMP_DOOR.attr.name="TEMP_DOOR";
	    item->TEMP_DOOR.attr.mode=00444;
	    item->TEMP_DOOR.show=show_temp;
	    item->TEMP_DOOR.store=NULL;
	device_create_file(&(item->dev),&(item->TEMP_DOOR));//++++++++++
	    item->ID_IN.attr.name="ID_IN";
	    item->ID_IN.attr.mode=00644;
	    item->ID_IN.show=show_temp_id;
	    item->ID_IN.store=store_temp_id;
	device_create_file(&(item->dev),&(item->ID_IN));//++++++++++
	    item->ID_OUT.attr.name="ID_OUT";
	    item->ID_OUT.attr.mode=00644;
	    item->ID_OUT.show=show_temp_id;
	    item->ID_OUT.store=store_temp_id;
	device_create_file(&(item->dev),&(item->ID_OUT));//++++++++++
	    item->ID_BOX.attr.name="ID_BOX";
	    item->ID_BOX.attr.mode=00644;
	    item->ID_BOX.show=show_temp_id;
	    item->ID_BOX.store=store_temp_id;
	device_create_file(&(item->dev),&(item->ID_BOX));//++++++++++
	    item->ID_DOOR.attr.name="ID_DOOR";
	    item->ID_DOOR.attr.mode=00644;
	    item->ID_DOOR.show=show_temp_id;
	    item->ID_DOOR.store=store_temp_id;
	device_create_file(&(item->dev),&(item->ID_DOOR));//++++++++++
	    item->comand.attr.name="comand";
	    item->comand.attr.mode=00600;
	    item->comand.show=show_comand;
	    item->comand.store=store_comand;
	device_create_file(&(item->dev),&(item->comand));//++++++++++
	    item->_auto.attr.name="_auto";
	    item->_auto.attr.mode=00444;
	    item->_auto.show=show_auto;
	    item->_auto.store=store_auto;
	device_create_file(&(item->dev),&(item->_auto));
	    item->LOG[0].attr.name="tmp";
	    item->LOG[0].attr.mode=00444;
	    item->LOG[0].show=show_logt;
	    item->LOG[0].store=NULL;
	device_create_file(&(item->dev),&(item->LOG[0]));
	    item->LOG[1].attr.name="log";
	    item->LOG[1].attr.mode=00444;
	    item->LOG[1].show=show_log;
	    item->LOG[1].store=NULL;
	device_create_file(&(item->dev),&(item->LOG[1]));

	    for (i=0;i<16;i++){
		item->out[i].attr.name=kmalloc(7,GFP_KERNEL);
		snprintf((char *)item->out[i].attr.name,7,"out_%02x",i);
		if(i<3)item->out[i].attr.mode=00644; else item->out[i].attr.mode=00666;
		item->out[i].show=show_out;
		item->out[i].store=store_out;
	    device_create_file(&(item->dev),&(item->out[i]));
		item->block[i].attr.name=kmalloc(9,GFP_KERNEL);
		snprintf((char *)item->block[i].attr.name,9,"block_%02x",i);
		item->block[i].attr.mode=00644;
		item->block[i].show=show_block;
		item->block[i].store=store_block;
	    device_create_file(&(item->dev),&(item->block[i]));
		item->TEMP[i].attr.name=kmalloc(7,GFP_KERNEL);
		snprintf((char *)item->TEMP[i].attr.name,7,"TEMP_%x",i);
		item->TEMP[i].attr.mode=00444;
		item->TEMP[i].show=show_temp;
		item->TEMP[i].store=NULL;
	    device_create_file(&(item->dev),&(item->TEMP[i]));
		item->ID[i].attr.name=kmalloc(5,GFP_KERNEL);
		snprintf((char *)item->ID[i].attr.name,5,"ID_%x",i);
		item->ID[i].attr.mode=00444;
		item->ID[i].show=show_temp_id;
		item->ID[i].store=NULL;
	    device_create_file(&(item->dev),&(item->ID[i]));
	    }
	return 1;
	}
    }
    return 0;
}
static void __exit dev_exit( void )
{
    struct mega1 *item;
    struct list_head *iter,*iter_safe;
    list_for_each_safe(iter,iter_safe,&list){
	item=list_entry( iter, struct mega1, list);
//	remove_pins(item);
	kfree(item->dev.platform_data);
	unregister_rs485_device(&(item->dev));
	list_del(iter);
	kfree(item);
    }
    unregister_rs485_driver(&mega8a1);
    printk( KERN_ALERT "rs485_dev: is unloaded!\n" );
}
static int __init dev_init( void )
{
    register_rs485_driver(&mega8a1);
    printk( KERN_ALERT "rs485_dev: loaded!\n" );
    return 0;
}

module_init( dev_init );
module_exit( dev_exit );
