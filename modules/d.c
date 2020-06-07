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
//
//
//  Using Semaphores in scull
//  =========================
//  Each daevice has a sem. Note it is init'd before scull made available
//  to the rest of system. i.e. init_MUTEX b4 scull_setup_cdev
//
//  if (down_interruptible(&dev->sem))
//      return -ERESTARTSYS;
//  typical way of dealing with failure.
//  higher layers of kernel will restart call or return error to user.
//  Should undo user-visible stuff before return the error.
//  otherwise if cannot undo things, do -EINTR
//
//
//  Reader Writer Semaphores
//  ========================
//  what if tasks only need to read protected data structures,
//  and some other tasks must make changes.
//  thus: could allow multiple concurrent readers, if they don't try to change.
//
//  rwsem <linux/rwsem.h>
//  struct rw_semaphore.
//  void init_rwsem(struct rw_semaphore *sem);
//
//  for r/o access
//  void down_read(struct rw_semaphore *sem);
//      may put calling process into an uninterruptible sleep
//  int down_read_trylock(struct rw_semaphore *sem);
//      won't wait if read access unavailable.
//      nonzero if access granted, 0 otherwise.
//      differs from most kernel fncs where success indicated by ret 0.
//  void up_read(struct rw_semaphore *sem);
//
//  for write access
//  void down_write(struct rw_semaphore *sem);
//  int down_write_trylock(struct rw_semaphore *sem);
//  void up_write(struct rw_semaphore *sem);
//  void downgrade_write(struct rw_semaphore *sem);
//      for case where writer lock needed for quick change, followed
//      by longer period of r/o access
//
//  possible to starve readers (when large number of writers who have priority
//    are waiting).
//  thus it's best to use when write access used rarely and for short periods
//  of time.
//
//
//
//  Completions
//  ===========
//  common pattern in kernel, initiate activity outside current thread
//  and wait for that activity to complete.
//  could be creation of a new kernel thread, 
//  or user-space process,
//  request to existing process,
//  or some hardware-based action
//
//  struct semaphore sem;
//  start_external_task(&sem);
//  down(&sem);
//
//  external task call up(&sem) when its work is done
//  but sem not best tool for this sitch
//
//  completion interface in 2.4.7 kernel <linux/completion.h>
//  DECLARE_COMPLETION(my_completion);
//  or dynamically
//  struct completion my_completion;
//  /* ... */
//  init_completion(&my_completion);
//
//  waiting for completion
//  void wait_for_completion(struct completion *c);
//
//  note this is an uninterruptible wait. if task never completed,
//  unkillable process
//
//  actual completion event may be signalled  by calling one of
//  void complete(struct completion *c);
//  void complete_all(struct completion *c);
//
//  if more than one thread waiting for same complet event,
//  complete wakes up one. complete_all allows all to proceed.
//
//  can reuse completion structure if no ambiguiity about what event being
//  signalled.
//  if complete_all used, must reinitialize completion before reusing it:
//  INIT_COMPLETION(struct completion c);
//
//  for an EXAMPLE, consider the complete module in example source.
//
//  typical use of completion mechanism is kernel thread termination
//  at module exit time.
//  some driver internal workins performed by kernel thread in while 1 loop.
//  when ready to be cleaned up, exit function tells thread to exit
//  and waits for completion.
//  kernel includes specific function to be used by the thread
//      void complete_and_exit(struct completion *c, long retval);
//
//
//
//  Spinlocks
//  =========
//  used in code that cannot sleep such as interrupt handlers.
//  higher general performance than semaphores when used properly.
//
//  <linux/spinlock.h>
//  spinlock_t my_lock = SPIN_LOCK_UNLOCKED;
//  or
//  void spin_lock_init(spinlock_t *lock);
//
//  obtain lock b4 critical section
//  void spin_lock(spinlock_t *lock);
//  Note: All spinlock waits are uninterruptible
//
//  release
//  void spin_unlock(spinlock_t *lock);
//
//
//
//  Spinlocks and Atomic Context
//  ============================
//  core rule for spinlocks is thay any code executed while holding
//  a spin lock must be atomic. Cannot sleep / relinquish processor.
//  Maybe not even to service interrupts.
//
//  Need to pay attention to every function that you call.
//
//  Must disable interrupts while spinlock is held (on local CPU only).
//  There are spinlock functions that automatically do this, but
//  wait until Chapter 10 for a complete discussion on interrupts.
//
//
//  More Spinlock Functions
//  =======================
//  4 functions that can lock a spinlock
//  void spin_lock(spinlock_t *lock);
//  void spin_lock_irqsave(spinlock_t *lock, unsigned long flags);
//  void spin_lock_irq(spinlock_t *lock);
//  void spin_lock_bh(spinlock_t *lock);
//
//  irqsave disables interrupts (on local processor) b4 taking lock, previous
//  interrupt state stored in flags (??? Should it be *flags ptr then?)
//
//  if are sure that should enable interrupts when release spinlock
//  (nothing else might've already disabled interrupts) then
//  spin_lock_irq and don't have to keep track of flags.
//
//  bh disables software interrupts but leaves hardware interrupts enabled
//
//  if using a spinlock running in sw/hw interrupt context, must use one of the
//  forms of spin_lock that disables interrupts.
//
//  4 ways to restore, corresponding to each of 4 ways to lock
//  void spin_unlock(spinlock_t *lock);
//  void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags);
//  void spin_unlock_irq(spinlock_t *lock);
//  void spin_unlock_bh(spinlock_t *lock);
//
//  flags arg passed to irqrestore must be same var passed to irqsave.
//  must call save and restore in same function otherwise code may break
//  on some architectures.
//
//  also are nonblocking spinlock operations (return nozero on success)
//  (return 0 if lock not obtained)
//  int spin_trylock(spinlock_t *lock);
//  int spin_trylock_bh(spinlock_t *lock);
//
//  
//
//  Reader/Writer Spinlocks
//  =======================
//  rwlock_t def'd in <linux/spinlock.h>
//
//  static
//  rwlock_t my_rwlock = RW_LOCK_UNLOCKED;
//  dynamic
//  rwlock_t my_rwlock;
//  rwlock_init(&my_rwlock);
//
//  for readers
//  void read_lock(rwlock_t *lock);
//  void read_lock_irqsave(rwlock_t *lock, unsigned long flags);
//  void read_lock_irq(rwlock_t *lock);
//  void read_lock_bh(rwlock_t *lock);
//
//  void read_unlock(rwlock_t *lock);
//  void read_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
//  void read_unlock_irq(rwlock_t *lock);
//  void read_unlock_bh(rwlock_t *lock);
//
//  no read_trylock
//
//  for writers
//  void write_lock(rwlock_t *lock);
//  void write_lock_irqsave(rwlock_t *lock, unsigned long flags);
//  void write_lock_irq(rwlock_t *lock);
//  void write_lock_bh(rwlock_t *lock);
//  int write_trylock(rwlock_t *lock);
//
//  void write_unlock(rwlock_t *lock);
//  void write_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
//  void write_unlock_irq(rwlock_t *lock);
//  void write_unlock_bh(rwlock_t *lock);
//
//  rw spinlocks can starve readers too.
//  not as common a problem, as if lock contention is bad enough to bring
//  starvation, performance is poor anyways.
//
//
//
//  Locking Traps
//  =============
//  https://ossi.sgi.com/projects/lockmeter  
//  TODO: come back here if get locking issues i guess
//
//
//
//  Lock-Free Algorithms
//  ====================
//  Circular Buffers
//  (must be careful to avoid pointers crossing eachother)
//  these show up relatively often in device drivers.
//  Net adaptors use circ buffs to exchange packets w/ processor.
//
//  as of 2.6.10 there is a generic circ buff implemenatation in kernel  
//  <linux/kfifo.h>
//
//
//
//  Atomic Variables (integers)
//  ================
//  sometimes shared resource is just a single integeer value.
//
//  kernel provides atomic integer type atomic_t in <asm/atomic.h>
//  not guaranteed to have more than 24 bits of int
//
//  operations:
//  atomic set, read, add, sub, inc, dec, incandtest, decandtest, subandtest,
//  addnegative,addreturn,subreturn,increturn,decreturn
//
//  note: operations requiring multiple atomic_t values require additional
//  locking
//  e.g.
//  ```
//  atomic_sub(amount, &first_atomic);
//  atomic_sub(amount, &second_atomic);
//  ```
//
//  Atomic Bit operations
//  =====================
//  <asm/bitops.h>
//  set bit, clear bit, change bit, test bit, test and set bit,
//  test and clear bit, test and change bit
//
//
//
//  Seqlocks
//  ========
//  blah blah blah
//
//
//
//  Read-Copy-Update
//  ================
//  optimized for common reads, and rare writes.
//  see creator whitepaper
//  http://www.rdrop.com/users/paulmck/rclock/intro/rclock_intro.html
//  <linux/rcupdate.h>
//
//
//
//
//
//
//
