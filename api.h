//extern struct device rs485_bus;
extern struct bus_type rs485_bus_type;


/*
 * The rs485 driver type.
 */

/*struct rs485_driver {
    char *version;
    struct module *module;
    struct device_driver driver;
    struct driver_attribute version_attr;
};

#define to_rs485_driver(drv) container_of(drv, struct rs485_driver, driver);

/*
 * A device type for things "plugged" into the LDD bus.
 */

/*struct rs485_device {
    char *name;
    struct rs485_driver *driver;
    struct device dev;
};

#define to_rs485_device(dev) container_of(dev, struct rs485_device, dev);
*/
//extern int register_rs485_device(struct rs485_device *);
//extern void unregister_rs485_device(struct rs485_device *);
//extern int register_rs485_driver(struct rs485_driver *);
//extern void unregister_rs485_driver(struct rs485_driver *);



extern char* to_bus(char* bus, char* address, char* comand, char* data, char* count);


