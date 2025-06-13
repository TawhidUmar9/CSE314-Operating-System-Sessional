#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(void)
{
    struct pstat ps;
    if (getpinfo(&ps) < 0)
    {
        printf("getpinfo failed\n");
        exit(1);
    }
    printf("PID\t|\tInUse\t|\tQueue\t|\tOrigTickets\t|\tCurrTickets\t|\tTimeSlices\n");
    for (int i = 0; i < NPROC; i++)
    {
        if (ps.pid[i])
        {
            printf("%d\t|\t%d\t|\t%d\t|\t%d\t\t|\t%d\t\t|\t%d\n",
                   ps.pid[i], ps.inuse[i], ps.inQ[i],
                   ps.tickets_original[i], ps.tickets_current[i],
                   ps.time_slices[i]);
        }
    }
    exit(0);
}