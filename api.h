#include <linux/device.h>

//extern struct device rs485_bus;
//extern struct bus_type rs485_bus_type;
#define BUFSIZE 250
extern int fas_rs485_bus(struct device *);
extern int rs485_infdev(struct device *);
extern int rs485_message_count(struct device *);
extern int rs485_register_read(struct device *, unsigned short first, unsigned char count);
extern int rs485_register_write(struct device *, unsigned short first, unsigned char count);
extern int rs485_register_write1(struct device *, unsigned short first);
extern int rs485_coil_write(struct device *, unsigned short address, unsigned char val);
extern int rs485_coil_read(struct device *, unsigned short address, unsigned short count);


extern int register_rs485_device(struct device *);
extern void unregister_rs485_device(struct device *);
extern int register_rs485_driver(struct device_driver *);
extern void unregister_rs485_driver(struct device_driver *);