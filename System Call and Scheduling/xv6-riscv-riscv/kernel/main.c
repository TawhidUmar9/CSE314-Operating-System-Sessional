#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "syscall.h"
#include "syscall_stat.h"
#include "spinlock.h"
volatile static int started = 0;

struct syscall_stat syscall_stats[50];


const char *syscall_names[] = {
    [SYS_fork] "fork",
    [SYS_exit] "exit",
    [SYS_wait] "wait",
    [SYS_pipe] "pipe",
    [SYS_read] "read",
    [SYS_kill] "kill",
    [SYS_exec] "exec",
    [SYS_fstat] "fstat",
    [SYS_chdir] "chdir",
    [SYS_dup] "dup",
    [SYS_getpid] "getpid",
    [SYS_sbrk] "sbrk",
    [SYS_sleep] "sleep",
    [SYS_uptime] "uptime",
    [SYS_open] "open",
    [SYS_write] "write",
    [SYS_mknod] "mknod",
    [SYS_unlink] "unlink",
    [SYS_link] "link",
    [SYS_mkdir] "mkdir",
    [SYS_close] "close",
    [SYS_history] "history",
    [SYS_settickets] "settickets",
    [SYS_getpinfo] "getpinfo",
};
// start() jumps here in supervisor mode on all CPUs.

void initialize_syscall_stat()
{
  for (int i = 1; i < NELEM(syscall_names); i++)
  {
    initlock(&syscall_stats[i].syscall_stat_lock, "syscall_stat_lock");
    strncpy(syscall_stats[i].syscall_name, syscall_names[i], sizeof(syscall_stats[i].syscall_name));
    syscall_stats[i].count = 0;
    syscall_stats[i].accum_time = 0;
  }
}
void main()
{
  if (cpuid() == 0)
  {
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging
    procinit();         // process table
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    plicinit();         // set up interrupt controller
    plicinithart();     // ask PLIC for device interrupts
    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    virtio_disk_init(); // emulated hard disk
    initialize_syscall_stat();
    userinit(); // first user process
    __sync_synchronize();
    started = 1;
  }
  else
  {
    while (started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  scheduler();
}
