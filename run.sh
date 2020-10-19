set -euxo pipefail
#!/bin/bash
KERNEL=../hello_modules/linux/arch/$(uname -m)/boot/bzImage
INITRAMFS=../hello_modules/mkroot/root.cpio.gz

    #cp modules/scull_load ../hello_modules/mkroot/output/host/root/home && \
make && \
    cp modules/*.ko ../hello_modules/modules/ && \
    (cd ../hello_modules && bash make.sh)


[ ! -f $KERNEL ] && echo "Uh oh, no kernel found at $KERNEL" && exit 1
[ ! -f $INITRAMFS ] && echo "Uh oh, no rootfs found at $INITRAMFS" && exit 2

# Special thanks to Landley for his mkroot project

qemu-system-x86_64 \
    -nographic \
    -no-reboot \
    -kernel $KERNEL \
    -initrd $INITRAMFS \
    -append "panic=1 console=ttyS0 "



