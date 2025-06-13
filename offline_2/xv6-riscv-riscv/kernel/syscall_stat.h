#include "spinlock.h"

struct syscall_stat {
    char syscall_name[16];
    int count;
    int accum_time;
    struct spinlock syscall_stat_lock;
};

extern struct syscall_stat syscall_stats[50];