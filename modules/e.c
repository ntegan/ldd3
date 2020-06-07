#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void) {
  printk(KERN_ALERT "hello, world\n");
  printk(KERN_DEBUG "Here I am: %s:%i\n", __FILE__, __LINE__);
  return 0;
}

static void hello_exit(void) { printk(KERN_ALERT "Goodbye, cruel world\n"); }

module_init(hello_init);
module_exit(hello_exit);

//  LDD3 Chapter 6: Advanced Char Driver Operations
//  https://static.lwn.net/images/pdf/LDD3/ch06.pdf
//
//  Extending on ch3 device driver that can synchronous read and write.
//  start with ioctl, then synchronizing w/ user space,
//  put procs to sleep/wake up, nonblocking i/o, ...
//
//
//
//  ioctl
//  =====
//  in userspace:
//  int ioctl(int fd, unsigned long cmd, ...);
//  dots represent single optional argument char *argp
//
//  ioctl driver method
//  int (*ioctl) (struct inode *inode, struct file *filp,
//                unsigned int cmd, unsigned long arg);
//  inode and filp correspond to fd passed by app, and are same parameters
//  passed to the open method.
//
//  ioctl implementations usually a big switch statement with
//  preprocessor definitions for each cmd number
//
//  want unique ioctl cmd numbers so don't accidentally send cmd
//  to wrong device fd. Error is EINVAL if send wrong command
//
//  check "include/asm/ioctl.h" (is this <asm/ioctl.h> ?)
//  and Documentation/ioctl-number.txt
//  header defines bitfields: type (magic num), ordinal num, xfer dir, arg size
//  ioctl-number.txt lists magic numbers used throughout kernel, so choose own
//
//  approved way to define ioctl cmd nums uses 4 bitfields
//  <linux/ioctl.h>
//  type: magic num 8 bits
//  number: ordinal (sequential number) 8 bits
//  direction: _IOC_NONE, _IOC_READ, _IOC_WRITE, _IOC_READ | _IOC_WRITE
//        data xfer as seen from application point of view
//        read is read from device => driver write to userspace
//  size: of user data involved. architecture dependend width but usually
//        13 / 14 bits. defined in macro _IOC_SIZEBITS
//
//  <asm/ioctl.h> included by <linux/ioctl.h>
//  defines macros to help setup command numbers as follows
//  _IO(type,nr) command no args
//  _IOR(type,nr,datatype) read data from driver
//  _IOW(type,nr,datatype) write data
//  _IOWR(type, nr, datatype) bidirectional xfers
//  size derived by applying sizeof to datatype argument
//
//  also can decode numbers:
//  _IOC_DIR(nr), _IOC_TYPE(nr), _IOC_NR(nr), _IOC_SIZE(nr)
//
//  e.g.
//  ```
//	#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC,   4)
//	#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC,  5, int)
//	#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC,  6, int)
//  ```
//
//  <linux/kd.h>
//  is example of old fashioned approach, using 16bit scalar values
//  to define ioctl cmds
//
//
//
//  Return Value
//  ============
//  what should be default for switch?
//  some say -EINVAL ("invalid argument")
//
//  but POSIX standard
//  -ENOTTY interpreted by clib as ("inappropriate ioctl for device")
//
//  
//
//  Predefined IOCTL commands
//  =================
//  decoded b4 my own file operations called
//  three groups
//  * can be issued on any file (regular, device, FIFO, socket)
//  * only on regular files
//  * specific to filesystem type
//
//  last group exec'd by implementation of hosting filesystem
//      (how `chattr` command works)
//
//  usually only care about first group (magic number 'T')
//
//
//
//  Using the IOCTL argument
//  ========================
//  address verification in <asm/uaccess.h>
//  int access_ok(int type, const void *addr, unsigned long size);
//  type={VERIFY_READ,VERIFY_WRITE} whetehr read user mem or write to user mem.
//  size byte count
//  if need R/W then use VERIFY_WRITE (verify both)
//
//  unlike most kernel functions, access_ok return bool val 1 success, 
//  0 failure. if false driver return -EFAULT to caller
//
//  first check _IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
//  then if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;
//
//  type is user oriented while access_ok is kernel-oriented so read/write
//  reversed
//  _IOC_DIR(cmd) & _IOC_READ) then err = !access_ok(VERIFY_WRITE,..)
//  else if _IOC_WRITE
//  if err return -EFAULT
//
//  optimizing for single datum transfers:
//  put_user(datum, ptr)
//  checks with access_ok, returns 0 on success, -EFAULT on error,
//  uses typeof and sizeof on the ptr argument to determine 1/2/4/8 bytes.
//
//  get_user(local, ptr);
//
//
//
//  Capabilities and Restricted Operations
//  ======================================
//  <linux/capability.h>
//  userspace syscalls capget and capset
//
//
//
//  scull ioctl command implementation
//  ==================================
//  ......
//
//
//  Device Control Without ioctl
//  ============================
//  have to make sure control sequences don't normally show up during
//  a regular data stream.
//
//  e.g. how ttys are controlled via escape sequences.
//  when ssh into a server, server can setup your terminal config by sending
//  you escape sequences.
//
//
//
//  Blocking I/O
//
//  Introduction to Sleeping
//  ========================
//  never sleep while running in an atomic context (e.g. in spin,seq,rcu lock).
//  be careful when sleep in semaphore lock hold.
//
//  wait queue allows a sleeping process to be found and woken up.
//  is a list of processes waiting for a specific event.
//  <linux/wait.h>
//  managed by a "wait queue heaad" structure of wait_queue_head_t
//
//  DECLARE_WAIT_QUEUE_HEAD(name);
//  or
//  wait_queue_head_t my_queue;
//  init_waitqueue_head(&my_queue);
//
//
//
//  Simple Sleeping
//  ===============
//  any process that sleeps must check to be sure that the
//  condition it was waiting for is really true when it wakes
//
//  wait_event(queue, condition);
//  wait_event_interruptible(queue, condition);
//  wait_event_timeout(queue, condition, timeout);
//  wait_event_interruptible_timeout(queue, condition, timeout);
//
//  queue is the wait queue head to use. passed by value.
//  condition is bool expr eval'd by macro before and after sleeping.
//  until condition evaluates to true, it sleeps again.
//
//  preferrd is wait event interruptible.
//  returns nonzero if sleep interrupted by a signal, and driver should 
//  return -ERESTARTSYS.
//
//  the timeout (expressed in jiffies) returns with a 0 if time expires,
//  regardless of condition
//
//  another thread (diff process, or interrupt handler maybe)
//  has to wakeup
//
//  void wake_up(wait_queue_head_t *queue);
//  void wake_up_interruptible(wait_queue_head_t *queue);
//
//  in examples, module called sleepy.
//
//
//
//
//  Blocking and Nonblocking Operations
//  ===================================
//
//  Sometimes want operations to not block, even if cannot completely finish.
//
//  Explicit nonblocking I/O indicated by O_NONBLOCK in filp->f_flags.
//  flag defined in <linux/fcntl.h> auto incl by <linux/fs.h>
//  gets its name from "open-nonblock", specified at open time.
//  
//  Normal blocking behavior:
//  process calls read, but no data yet available then the process blocks.
//  process awakened as soon as some data arrives, and that data is returned
//  even if data is less than amount requested in count argument to the method.
//
//  process calls write, no space in buffer. process must block
//  on a different wait queue than the wait queue used for reading.
//  when some data written to the hardware device, and space free in the
//  output buffer, process awaken and write call succeeds although data
//  may only be partially written if isn't room in the buffer for the requested
//  count bytes.
//
//  these assume that there are both in and output buffers.
//  input buffer required to avoid losing data arriving when nobody is reading.
//  data cannot be lost on write, but output buff useful for better hw perfmnce
//  due to reduced number of context switches and user/kernel transitions.
//
//  Read and write behavior of O_NONBLOCK specified.
//  calls simply return -EAGAIN (try it again) if read and no data available
//  or if write and no space in the buffer
//
//  can also make a blocking/nonblocking open for device initialization.
//
//  only read,write,open are affected by O_NONBLOCK
//
//
//
//  A Blocking I/O Example
//  ======================
//  leftoff here TODO: p19/48
//
//
//
