#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

void test(void) {
  struct file_operations scull_fops = {
      .owner = THIS_MODULE,
      //.llseek = scull_llseek,
      //.read =   scull_read,
      //.write = scull_write,
      //.ioctl = scull_ioctl,
      //.open = scull_open,
      //.release = scull_release,
  };
}

static int hello_init(void) {
  test();
  printk(KERN_ALERT "hello, world\n");
  return 0;
}

static void hello_exit(void) { printk(KERN_ALERT "Goodbye, cruel world\n"); }

module_init(hello_init);
module_exit(hello_exit);

//  LDD3 Chapter 3: Char Drivers
// https://static.lwn.net/images/pdf/LDD3/ch03.pdf
//
//  SCULL
//  =====
//  simple character utility for loading localities
//  acts on a memory area as though it were a device.
//
//  scull implements following devices. each kind is called a "type"
//
//  scull0 to scull3
//  each is global and persistent.
//  fun to work with, accessed and tested using
//  conventional cp,cat,shellIO redirection
//
//  scullippe0 to scullpipe3
//  4 FIFO devices. 1 process reads what another writes. if multiple read
//  the device, they contend for data. This shows how
//  blocking and non-blocking read/write can be implement without resort
//  to interrupts. real drivers use hw interrupts (ch10), but
//  blocking and nonblocking is important topic t oo
//
//  scullsingle
//  scullpriv
//  sculluid
//  scullwuid
//  blablablah
//
//  This chapter covers scull0-3
//
//  major minor numbers
//  ===================
//  in /dev, special files char devices have 'c' in first col of ls -l
//  block devices have a 'b'
//
//  typically the major number IDs the driver associated with the device
//
//  dev_t type (<linux/types.h>)
//  holds device numbers (major + minor)
//  obtain parts: MAJOR(dev_t dev); MINOR(dev_t dev);
//  or other way: MKDEV(int major, int minor);
//
//  <linux/fs.h>
//  int register_chrdev_region(dev_t first,unsigned int count, char *name);
//  first is beginning device number of range want to allocate
//  first minor number often 0. count is num contiguous devnums u want
//  name is name of device, appear in /proc/devices and sysfs
//  return 0 if successful error returns negative err code
//
//  or without knowing what first
//  int alloc_chrdev_region(dev_t *dev, unsigned int firstminor,
//            unsigned int count, char *name);
//  here dev is output only param. holds first num in your alloc range
//  firstminor usually 0
//
//  free with
//  void unregister chrdev_region(dev_t first, unsigned int count);
//  in cleanup function
//
//
//
//  Major number dynamic allocation
//  ===============================
//  som emajor dev nums statically alloc'd to common devices
//  Documentation/devices.txt
//
//  issue with dynamic is can't make device nodes in advance,
//  maj num will vary.
//  hardly a problem because once num assigned, can read from /proc/devices
//
//  to load a driver using a dynamic maj num, invocation of insmod
//  can be replaced y a simple script that calls insmod,
//  reads /proc/devices in order to create special file(s).
//  ```
//  Character devices:
//   1 mem
//   2 pty
//  .....
//   180 usb
//
//  Block Devices:
//   2 fd
//   ...
//   66 fd
//  ```
//
//  See b_init.sh for sculls script they use to make device nodes
//  a scull_unload is also available to unload the script.
//  this script may be used in rc.local for startup, or manually.
//
//
//
//
//  Important Data Structures
//  =========================
//  file_operations, file, inode
//
//
//
//  File Operations
//  ===============
//  <linux/fs.h>
//  structure is a collection of fnc pts.
//  each file (represented by `file` struct) contains a f_op that
//  points to a file perations structure.
//  THe operations mostly in charge implement syscalls (open, read,..)
//
//  the file is an "object" and functions operating on it are its "methods"
//  (obj oriented prgramming)
//
//  the fops: field in stuct must point to function in driver that
//  implementts a specific peration, or NULL.
//
//  PAGE 50
//  3 page long list of possible file operations
//
//  See code abve for how they initialize their scull_fops
//  file_operations struct
//
//
//
//  file Structure
//  ==============
//  defined in <linux/fs.h>
//  struct file
//  represents an open file. Every open file in the system has an associated
//    struct file in kernel space.
//  created by kernel on `open`, passed to functions operating on file,
//  until last `close`
//
//  pointer to struct file usually called `filp` (filepointer)
//
//  PAGE 53-54
//  list of struct fields
//  has a mode, for r/w
//  and fflags which als has r/w. all flags defined in <linux/fcntl.h>
//    but use mode for r/w rather than f_flags.
//  has f_op, struct file_operations*
//  has struct dentry *f_dentry. filp->f_dentry->d_inode is inode struct
//
//
//
//  inode Structure
//  ===============
//  one file structure per open descriptor.
//  can have multiple open descriptors per file, but all point to a
//  single inode structure.
//
//  contains
//  dev_t i_rdev. (actual device number)
//  struct cdev *i_cdev. kernel's internal structure representing char devs.
//      contains pointer to a that struct when inode referes to a char dev file.
//
//      unsigned int iminor (struct inode *inode);
//      unsigned int imajor (struct inode *inode);
//
//
//  Char Device Registration
//  ========================
//  kernel represents char devices with `struct cdev`
//  <linux/cdev.h>
//
//  two ways of alloc+initialize
//
//  if want to obtain standalone cdev struct at runtime
//  struct cdev *my_cdev = cdev_alloc();
//  my_cdev->ops = &my_fops;
//
//  Or probably want to embed cdev structure in a device-specific structure.
//  in this case, init structure that already allocated with
//  void cdev_init(struct cdev *cdev, struct file_peration *fops);
//
//  then for both, need to set
//  cdev->owner = THIS_MODULE;
//
//  then once setup, tell kernel about it with call to
//  int cdev_add(struct cdev *dev, dev_t num, unsigned int count);
//
//  if returns negative, device has not been  added.
//  don't call cdev add until driver ready to handle operations on the device.
//
//  to remove:
//  void cdev_del (struct cdev *dev);
//
//
//
//
//
//  Open and Release
//  ================
//  Driver open should :
//  check for device specific errors,
//  init device if opened fr 1st time,
//  update f_op ptr if need be,
//  allocate data struct fr filp->private_data,
//
//  int (*open)(struct inode *inode, struct file *filp);
//  inode has i_cdev field.
//  we don't want cdev, we want struct scull_dev which contains cdeevv.
//
//  `container_of` macro defined in <linux/kernel.h>
//  container_of(pointer, container_type, container_field);
//
//  e.g. struc scull_dev *dev;
//    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
//    filp->private_datat = dev;    // for other "meethodds"
//
//  they don't keep an open count for their sculls.
//
//
//  Release Method
//  deallocate anything open allocated in filp->private data,
//  shut down device on last close.
//
//
//  Scull/Linux Memory Usage
//  ========================
//  <linux/slab.h>
//  void *kmalloc(size_t size, int flags);
//  void kfree(void *ptr);
//  flags in detail ch8, now just use GFP_KERNEL
//
//  is legal to free(NULL);
//
//  kmalloc not most efficient for large areas of memory (see ch8)
//
//  writee a single byte in scull consumes 8000-12000 thousand bytes of mem.
//  4k for quantum, 4/8k for qnautum set depending ptr 32 bits/64 bits.
//
//
//
//  Read and Write
//  ==============
//  ssize_t read (struct file *filp, char __user *buff,
//                  size_t count, loff_t *offp);
//  ssize_t write (struct file *filp, const char __user *buff,
//                  size_t count, loff_t *offp);
//
//  See mre functions in "Using the ioctl Argument" in ch1?
//  <asm/uaccess.h>
//  unsigned long copy_to_user(void __user *to,
//                    const void *from, unsigned long count);
//  unsigned long copy_from_user(void *to,
//                    const void __user *from, unsigned long count);
//
//  if invalid address encountered during copy, only part of data is copied.
//  both cases: return value is amount of memory still to be copied.
//  scull code returns -EFAULT to user if it doesn't return 0.
//
//  depending on amount of data transferred, should update the file position
//  at *offp to represent current file position after successful syscall.
//
//  pread and pwrite syscalls operate frm a given file offset and do not change
//  the file position as seen by other system calls.
//
//  read and write return negative val if error.
//  ret >=0 indicates how many bytes successfully transferred
//
//
//  Read
//  ====
//  read ret value interpreted by calling app program.
//  if val === count passed t read, yes all bytes transferred.
//  if positibe but < count, only part data transferred.
//  if value == 0, EOF.
//  negative => error, <linux/errno.h>
//
//
