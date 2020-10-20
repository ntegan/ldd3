
#include "0.h"

#include <linux/cdev.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define KMOD_NUM_MINORS 1
#define KMOD_DEV_NAME "/dev/egan"

MODULE_LICENSE("Dual BSD/GPL");

typedef struct {
  dev_t my_dev;
  struct cdev my_cdev;

} kmod_ctx;

kmod_ctx my_ctx;

/*
 * uaccess
unsigned long copy_to_user(void __user *to, const void *from,
                           unsigned long count);
unsigned long copy_from_user(void *to, const void __user *from,
                             unsigned long count);

* slab
void *kmalloc(size_t size, int flags);
void kfree(void *ptr);
*/
ssize_t fops_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos) {
  char byte = 0x69;
  int i;
  for (i = 0; i < count; i++) {
    copy_to_user(buf + count, &byte, 1);
  }
  // return -EFAULT;  // if copy_{}_user is not 0
  printk(KERN_INFO "egan: gave: %ld", count);
  return count;
}
ssize_t fops_write(struct file *filp, const char __user *buf, size_t count,
                   loff_t *f_pos) {
  // return -EFAULT;  // if copy_{}_user is not 0
  printk(KERN_INFO "egan: got: %ld", count);
  return count;
}
int do_something(unsigned long arg) {
  if (access_ok(arg, sizeof(int)) == 1) {
    // yay we can read an ints worth of data from arg

    return 0;
  }
  return -1;
}
unsigned long count_number_of_processes(void) {
  struct task_struct *task = &init_task;
  unsigned long num_tasks = 0;

  do {
    num_tasks++;
    task = list_entry(task->tasks.next, struct task_struct, tasks);
  } while (task != &init_task);
  return num_tasks;
}
int do_process_info_request_first_time(unsigned long arg) {
  unsigned int num_procs = count_number_of_processes();

  return 0;
}
int do_process_info_request_second_time(unsigned long arg) { return 0; }
int do_process_info_request(unsigned long arg) {
  // arg is kmod_process_request*

  unsigned long num_processes_requested = 0;
  kmod_process_request *kpr;
  if (access_ok(arg, sizeof(kmod_process_request)) == 0) {
    printk(KERN_ALERT "egan: definitely invalid\n");
    // definitely invalid
    return -1;
  }
  kpr = (kmod_process_request *)vmalloc(1 * sizeof(kmod_process_request));
  if (!kpr) return -3;
  if (0 != copy_from_user(kpr, (void *)arg, 1 * sizeof(kmod_process_request))) {
    // still some bytes left to copy
    vfree(kpr);
    return -2;
  }
  copy_to_user(kpr->p_process_infos, "HELLO\n\0", 7);
  vfree(kpr);
  printk(KERN_ALERT "egan: user requested %ld processes\n",
         kpr->num_process_infos_requested);
  if (kpr->num_process_infos_requested == 0) {
    return do_process_info_request_first_time(arg);
  } else {
    return do_process_info_request_second_time(arg);
  }
}
long fops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
  switch (cmd) {
    case KMOD_IOA:
      if (do_something(arg)) {
        return -EFAULT;
      }
      break;
    case KMOD_IOB:
      break;
    case KMOD_IOC:
      break;
    case KMOD_IOD:
      // rw
      // arg is usr
      if (do_process_info_request(arg)) {
        return -EFAULT;
      }
      break;
    default:
      return -ENOTTY;
      break;
  }
  return 0;
}
int fops_open(struct inode *inode, struct file *filp) {
  // initialization prepare for later operations.
  // allocate/fill any data struct needed to be put in filp->private_data

  // first, id which device being opened
  // inode->i_cdev contains cdev we setup before
  // TODO: try container_of here
  // filp->private_data = &my_ctx;
  printk(KERN_INFO "egan: opened");
  return 0;
}
int fops_release(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "egan: released");
  return 0;
}
struct file_operations fops = {.owner = THIS_MODULE,
                               .llseek = NULL,
                               .read = fops_read,
                               .write = fops_write,
                               .unlocked_ioctl = fops_ioctl,
                               .open = fops_open,
                               .release = fops_release

};

static int ___init(void) {
  char banner[] =
      "\n  ___   __ _   __ _  _ __  \n"
      " / _ \ / _` | / _` || '_ \ \n"
      "|  __/| (_| || (_| || | | \n"
      " \___| \__, | \__,_||_| |_\n"
      "        |___/              \n ";
  printk(KERN_INFO "%s", banner);

  if (alloc_chrdev_region(&my_ctx.my_dev, 0, KMOD_NUM_MINORS, KMOD_DEV_NAME)) {
    return -1;  // TODO return E_FAILED_TO_INIT
  };

  // struct file filp; // represents an open file

  // struct inode inood; // unique to a file on disk. can have many filps open

  // struct cdev *my_cdev = cdev_alloc();
  cdev_init(&my_ctx.my_cdev, &fops);
  my_ctx.my_cdev.owner = THIS_MODULE;
  my_ctx.my_cdev.ops = &fops;

  // TODO: is this 1 same as above? --> num minors?
  if (cdev_add(&my_ctx.my_cdev, my_ctx.my_dev, 1)) {
    unregister_chrdev_region(my_ctx.my_dev, KMOD_NUM_MINORS);
    return -1;  // TODO: error code
  }
  printk(KERN_ALERT "egan: successful dev init\n");

  struct task_struct *task;
  /*
  for (task = current; task != &init_task; task = task->parent) {
    printk(KERN_INFO "egan: pid %d", task->pid);
  }
  // task is nw init task
  */
  task = &init_task;
  do {
    // printk(KERN_INFO "egan: %d\t%s\n", task->pid, task->comm);
    task = list_entry(task->tasks.next, struct task_struct, tasks);
  } while (task != &init_task);
  return 0;
}

static void ___exit(void) {
  cdev_del(&my_ctx.my_cdev);
  unregister_chrdev_region(my_ctx.my_dev, KMOD_NUM_MINORS);
  printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(___init);
module_exit(___exit);
