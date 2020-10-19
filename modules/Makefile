ifneq ($(KERNELRELEASE),)

#kbuild
obj-m := 0.o
#obj-m := a.o b.o c.o d.o scull.o 0.o
#scull-objs := scull/main.o scull/pipe.o scull/access.o
#scull-objs := scull/main.o scull/pipe.o scull/access.o

#a-y := one.o two.o

else
#normal makefile
# compiling for compiled updstream kernel
#KDIR ?= ../../hello_modules/linux

# compiling for my running kernel
KDIR ?= /lib/modules/5.9.1-arch1-1/build/

default:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	rm -f *.mod *.mod.c *.mod.o *.o *.a *.order *.symvers .*.cmd
clean2: clean
	rm -f *.ko

endif
