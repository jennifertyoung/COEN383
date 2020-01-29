#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scheduling_algorithm_api.h"

int do_fcfs(job * job_array, int num_jobs)
{
    int i = 0;
    int quanta = 0;
    for (i = 0; quanta < 100 & i < num_jobs; ++i)
    {
        // wait if the current quanta is not busy and no other job has arrived
        while (quanta < (int)ceil(job_array[i].arrival_time))
            ++quanta;
        printf("Starting quanta for process %d: %d\n", i, quanta);
        while (job_array[i].done == 0)
        {
            sched_job_at_quantum(i, quanta);
            ++quanta;
        }
    }
    return 0;
}
