#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define CHILDREN 4
#define LOOP_COUNT 10000
#define SLEEP_INTERVAL 10
#define SLEEP_TIME 1

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "Usage: %s <ticket_number>\n", argv[0]);
        exit(1);
    }

    int tickets = atoi(argv[1]);

    if (settickets(tickets) < 0)
    {
        fprintf(2, "Error: settickets(%d) failed\n", tickets);
        exit(1);
    }

    for (int i = 0; i < CHILDREN; i++)
    {
        if (fork() == 0)
        {
            volatile int counter = 0;
            for (int j = 0; j < LOOP_COUNT; j++)
            {
                counter++;
                if (j % SLEEP_INTERVAL == 0)
                    sleep(SLEEP_TIME);
            }
            exit(0);
        }
    }

    volatile int counter = 0;
    for (int j = 0; j < LOOP_COUNT; j++)
    {
        counter++;
        if (j % SLEEP_INTERVAL == 0)
            sleep(SLEEP_TIME);
    }

    for (int i = 0; i < CHILDREN; i++)
    {
        wait(0);
    }

    exit(0);
}
