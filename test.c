#include <linux/init.h>
#include <linux/module.h>
#include "api.h"
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Andrey Fokin <foanse@gmail.com>" );
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
