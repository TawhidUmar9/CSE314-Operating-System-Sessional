#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/syscall.h"
#include "kernel/syscall_stat.h"

void print_history(int syscall_num)
{
    struct syscall_stat s_stat;

    if (history(syscall_num, &s_stat) == 0)
    {
        printf("%d: syscall: %s, #: %d, time: %d\n",
               syscall_num,
               s_stat.syscall_name,
               s_stat.count,
               s_stat.accum_time);
    }
    else
    {
        printf("Error fetching history for syscall %d\n", syscall_num);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        // No arguments, print all
        for (int i = 1; i <= 24; i++)
            print_history(i);
    }
    else
    {
        int syscall_num = atoi(argv[1]);
        if (syscall_num > 0)
        {
            print_history(syscall_num);
        }
        else
        {
            printf("Invalid syscall number.\n");
        }
    }

    exit(0);
}