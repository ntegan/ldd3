#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

#define KMOD_IOC_MAGIC 'f'

// no data xfer?
#define KMOD_IOA _IO(KMOD_IOC_MAGIC, 0)

// read data from the driver
#define KMOD_IOB _IOR(KMOD_IOC_MAGIC, 0, long unsigned)
#define KMOD_IOC _IOW(KMOD_IOC_MAGIC, 0, long unsigned)
#define KMOD_IOD _IOWR(KMOD_IOC_MAGIC, 0, int)

// KMOD_IOD gets process info
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

#define MY_COMM_LEN (TASK_COMM_LEN * 2)
typedef struct {
  unsigned long pid;
  unsigned long p_mm;
  char comm[MY_COMM_LEN];
  int should_get_vm_areas;
  // pid_t pid;
  // mm_struct *p_mm;
} kmod_process_info;
typedef struct {
  unsigned long num_process_infos_requested;
  unsigned long num_process_infos_fulfilled;
  kmod_process_info *p_process_infos;
} kmod_process_request;
