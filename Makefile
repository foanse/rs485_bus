CURRENT = "4.4.39-v7"
#CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/misc

#EXTRA_CFLAGS += -E

TARGET1 = rs485_bus
#TARGET2 = test
TARGET3 = tiny2313a_1_1
#obj-m	:= $(TARGET1).o $(TARGET2).o $(TARGET3).o
obj-m	:= $(TARGET1).o $(TARGET3).o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	cp -v $(TARGET).ko $(DEST)
	/sbin/depmod -v | grep $(TARGET)
	/sbin/insmod $(TARGET).ko
	/sbin/lsmod | grep $(TARGET)

uninstall:
	/sbin/rmmod $(TARGET)
	rm -v $(DEST)/$(TARGET).ko
	/sbin/depmod

clean:	
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~
	@rm -fR .tmp*
	@rm -rf .tmp_versions

