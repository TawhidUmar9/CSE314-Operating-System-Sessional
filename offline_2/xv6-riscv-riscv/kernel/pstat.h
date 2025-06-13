#ifndef _PSTAT_H_
#define _PSTAT_H_
#include "param.h"
#include "spinlock.h"
struct pstat
{
    int pid[NPROC];
    int inuse[NPROC];
    int inQ[NPROC];
    int tickets_original[NPROC];
    int tickets_current[NPROC];
    int time_slices[NPROC];
    struct spinlock lock;
};

#endif // _PSTAT_H_