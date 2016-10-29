#include <linux/device.h>

//extern struct device rs485_bus;
//extern struct bus_type rs485_bus_type;

extern int fas_rs485_bus(struct device *);


extern int register_rs485_device(struct device *);
extern void unregister_rs485_device(struct device *);
extern int register_rs485_driver(struct device_driver *);
extern void unregister_rs485_driver(struct device_driver *);