#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "syscall_stat.h"
#include "pstat.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_history(void)
{
  int syscall_num;
  uint64 userspace_struct_address;

  argint(0, &syscall_num);
  argaddr(1, &userspace_struct_address);
  if (syscall_num < 0 || syscall_num >= NELEM(syscall_stats))
  {
    return -1;
  }
  struct syscall_stat temp_stat;
  // acquire lock
  acquire(&syscall_stats[syscall_num].syscall_stat_lock);
  // temp_stat = syscall_stats[syscall_num];
  memmove(&temp_stat, &syscall_stats[syscall_num], sizeof(temp_stat));
  release(&syscall_stats[syscall_num].syscall_stat_lock);
  // Ensure the syscall name is null-terminated
  temp_stat.syscall_name[sizeof(temp_stat.syscall_name) - 1] = '\0';
  // Check if the userspace address is valid
  if (userspace_struct_address + sizeof(temp_stat) >= MAXVA)
  {
    return -1;
  }
  // Copy the syscall statistics to user space
  int ret_val = copyout(myproc()->pagetable, userspace_struct_address, (char *)&temp_stat, sizeof(temp_stat));
  if (ret_val < 0)
  {
    return -1;
  }
  return 0;
}

// MLFQ scheduler related syscalls
uint64
sys_settickets(void)
{
  int tickets;
  struct proc *p = myproc();

  argint(0, &tickets);
  if (tickets < 1)
  {
    acquire(&p->lock);
    p->tickets_original = DEFAULT_TICKET_COUNT;
    p->tickets_current = DEFAULT_TICKET_COUNT;
    release(&p->lock);
    return -1;
  }

  acquire(&p->lock);
  p->tickets_original = tickets;
  p->tickets_current = tickets;
  release(&p->lock);

  return 0;
}

uint64
sys_getpinfo(void)
{
  struct pstat pstat_local;
  struct proc *p;

  uint64 uptr;
  argaddr(0, &uptr);

  // Initialize and fill pstat_local
  for (int i = 0; i < NPROC; i++)
  {
    pstat_local.pid[i] = 0;
    pstat_local.inuse[i] = 0;
    pstat_local.inQ[i] = 0;
    pstat_local.tickets_original[i] = 0;
    pstat_local.tickets_current[i] = 0;
    pstat_local.time_slices[i] = 0;
    initlock(&pstat_local.lock, "pstat_lock");
  }

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state != UNUSED)
    {
      acquire(&pstat_local.lock);
      int i = p - proc;
      pstat_local.pid[i] = p->pid;
      pstat_local.inuse[i] = 1;
      pstat_local.inQ[i] = p->queue;
      pstat_local.tickets_original[i] = p->tickets_original;
      pstat_local.tickets_current[i] = p->tickets_current;
      pstat_local.time_slices[i] = p->time_slices;
      release(&pstat_local.lock);
    }
    release(&p->lock);
  }

  acquire(&pstat_local.lock);
  int ret = copyout(myproc()->pagetable, uptr, (char *)&pstat_local, sizeof(struct pstat));
  release(&pstat_local.lock);
  if (ret < 0)
  {
    return -1;
  }

  return 0;
}