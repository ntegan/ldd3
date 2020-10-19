#include <linux/ioctl.h>

#define KMOD_IOC_MAGIC 'f'

#define KMOD_IOA _IO(KMOD_IOC_MAGIC, 0)
// read data from the driver
#define KMOD_IOB _IOR(KMOD_IOC_MAGIC, 0, long unsigned)
#define KMOD_IOC _IOW(KMOD_IOC_MAGIC, 0, long unsigned)
#define KMOD_IOD _IOWR(KMOD_IOC_MAGIC, 0, int)
