#include <linux/init.h>
#include <linux/module.h>

// Usage: insmod a.ko howmany=10 whom="Mom"

MODULE_LICENSE("Dual BSD/GPL");

static char *whom = "default";
static int howmany = 2;
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);

static int hello_init(void) {
  printk(KERN_ALERT "hello, world\n");
  return 0;
}

static void hello_exit(void) { printk(KERN_ALERT "Goodbye, cruel world\n"); }

module_init(hello_init);
module_exit(hello_exit);

//  LDD3 Chapter 2: compiling modules or something
// https://static.lwn.net/images/pdf/LDD3/ch02.pdf

//  Current Process
//  ===============
// Kernel code can refer to current process global item `current`
// defined in <asm/current.h>, ptr to struct task_struct
//  defined in <linux/sched.h>
//
//  printk (KERN_INFO "The process is \"%S\" (pid %i)\n",
//      current->comm, current->pid);

//  Version
//  =======
//  <linux/version.h>  auto included by <linux/module.h>
//  UTS_RELEASE = e.g. "2.6.10"
//  LINUX_VERSION_CODE e.g. 2.6.10 is 132618 (0x02060a)
//  KERNEL_VERSION(major,minor,release) macro build integer version code
//    e.g. KERNEL_VERSION(2,6,10) == 132618

//  Module Stacking
//  ===============
//  insmod resolves undefined symbols against table of public kernel
//  symbols. when module loaded, exported symbols become part of the table.
//
//  allows for module stacking. e.g. `msdos` filesystem relies on
//  symbols exported by the `fat` module,
//  USB input device module stacks on `usbcore` and `input` modules
//
//  EXPORT_SYMBOL(name);
//  EXPORT_SYMBOL_GPL(name);
//  (details of how this works are in <linux/module.h>
//
//
//  Includes
//  ========
//  <linux/moduleparam.h> enables passing of parameters to module
//  at load time
//
//  shuld specify MODULE_LICENSE("GPL");
//  GPL GPL v2 "GPL and additional rights", "Dual BSD/GPL", "Dual MPL/GPL",
//  "Proprietary". Proprietary taints the kernel, and is enabled by default
//
//  MODULE_AUTHOR who wrote, MODULE_DESCRIPTION,
//  MODULE_VERSION (see linuxmodule.h for version string conventions)
//  MODULE_ALIAS (aka for this module another name)
//  MODULE_DEVICE_TABLE (tell userspace which devices this mod suppports)
//
//
//  Initialization
//  ==============
//  init function
//  static int __init i_f(void) {}
//  module_init(i_f);
//  static cause not meant to be visible outside the specific file.
//
//  cleanup function
//  static void __exit c_f(void) {}
//  module_exit(c_f);
//
//  Moduleparameters
//  ================
//  See top of file for usage and parameter implementation
//  Can also do module_param_array(name, type, num, perm)
//  name is name of array (and of the parameter),
//  type is type of array elems,
//  num is integer var,
//  perm is usual permissions value. if array param set at load time,
//  num set to number of values supplied.
//
//  permission values found in <linux/stat.h>
