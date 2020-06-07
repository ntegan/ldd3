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

//  LDD3 Chapter 5: Concurrency and Race Conditions
//  https://static.lwn.net/images/pdf/LDD3/ch05.pdf
//
//  You're "contrarin" me
//
//  Pitfalls in scull
//  =================
//  Race condition when two processes both attempting to write to same
//  offset in the same scull device
//  ```
//  if (!dptr->data[s_pos]) {
//  dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
//  ....
//  ```
//  both processes can do if statement, then alloc memory, then 
//  memory gets leaked, and ...
//
//
//
//
//  Concurrency and its Management
//  ==============================
//  Kernel code is preemptible. device interrupts can cause concurrent exec
//  of module code.
//
//  kernel provides mechanisms for delayed code execution:
//  workqueues, tasklets, timers,
//
//
//
//  Semaphores and Mutexes
//  ======================
//  it is possible for kmalloc to block/ the calling thread to sleep while
//  waiting.
//  
//  For this purpose, semaphore is the best choice.
//  Process wishing to enter crit section calls P,
//      if sem val > 0, it is decremented, and process continues.
//      if sem <=0, it waits
//  Unlock semaphore by calling V, incrementing val of semaphore, and wake up
//      processes that are wating
//
//  Mutex semaphores are initially set to 1.
//
//
//
//  Linux Semaphore Implementation
//  ==============================
//  <asm/semaphore.h>
//  struct semaphore.
//  one way to create is to create a semaphore directly and set it up with:
//    void sema_init(struct semaphore *sem, int val);
//  where val (mutex=1) is initial val to assign to the semaphore
//
//  to make it easier (mutexes are most common use of semaphore), use
//  DECLARE_MUTEX(name);        // init'd to 1
//  DECLARE_MUTEX_LOCKED(name); // init'd to 0
//  or
//  void init_MUTEX(struct semaphore *sem);
//  void init_MUTEX_LOCKED(struct semaphore *sem);
//
//  In Linux, P function is called down. (decrements the sem value)
//  3 versions of down
//  void down(struct semaphore *sem);
//      waits as long as need be
//  int down_interruptible(struct semaphore *sem);
//      almost always the one want. allows user-space process waiting on sem
//      to be interrupted by user.
//      ** If operation is interrupted, func returns nonzero and caller
//      does not hold the semaphore.
//      Must check the ret value to properly use it
//  int down_tryloc(struct semaphore *sem);
//      This never sleeps. if not available, return immediately with nonzero
//      return value.
//
//  Linux equivalent of V is up
//  void up(struct semaphore *sem);
//
