#include <linux/device.h>

//extern struct device rs485_bus;
//extern struct bus_type rs485_bus_type;

extern int register_rs485_device(struct device *);
extern void unregister_rs485_device(struct device *);
extern int register_rs485_driver(struct device_driver *);
extern void unregister_rs485_driver(struct device_driver *);



extern char* to_bus(char* bus, char* address, char* comand, char* data, char* count);


