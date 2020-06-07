ifneq ($(KERNELRELEASE),)

#kbuild
obj-m := a.o b.o c.o scull.o
scull-objs := scull/main.o scull/pipe.o scull/access.o

#a-y := one.o two.o

else
#normal makefile
KDIR ?= ../../hello_modules/linux

default:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	rm -f *.mod *.mod.c *.mod.o *.o *.a *.order *.symvers .*.cmd
clean2: clean
	rm -f *.ko

endif
