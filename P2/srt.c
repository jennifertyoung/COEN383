#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scheduling_algorithm_api.h"

// Helper function to compare the remaining times of two jobs
int compare_remaining_times(const void * a, const void * b)
{
    job *j1 = (job*)a;
    job *j2 = (job*)b;

    // calculate the remaining time for the two jobs
    float j1_rem_time;
    float j2_rem_time;

    get_remaining_run_time(j1->jobnum, &j1_rem_time);
    get_remaining_run_time(j2->jobnum, &j2_rem_time);

    // compare the remaining times for the two jobs
    float remaining_time_delta = j1_rem_time - j2_rem_time;
    if (remaining_time_delta > 0)
      return 1;
    if (remaining_time_delta < 0)
      return -1;

    return remaining_time_delta;
}

int do_srt(job * job_array, int num_jobs)
{
    int i = 0;
    int qi = 0;
    int target = -1;
    float rem_time = 0;
    int max_quantum = get_quantum_stop_scheduling_value();
    printf("Max Quantum: %d \n", max_quantum);
    for (qi = 0; qi <= max_quantum; ++qi)
    {
        // copy job_array to temp array
        job temp[num_jobs];
        for(i = 0; i < num_jobs; i++)
        {
          temp[i] = job_array[i];
        }
        // sort temp array
        qsort(temp, num_jobs, sizeof(job), compare_remaining_times);

        // target = job with SRT that has arrived and is not yet completed
        i = 0;
        target = -1;
        do
        {
          get_remaining_run_time(temp[i].jobnum, &rem_time);
          if (qi >= temp[i].arrival_time && rem_time > 0)
          {
            target = temp[i].jobnum;
            sched_job_at_quantum(target, qi);
          }
          ++i;
        } while (i < num_jobs && target == -1);
      }

      do
      {
          target = -1;
          // copy job_array to temp array
          job temp[num_jobs];
          for(i = 0; i < num_jobs; i++)
          {
            temp[i] = job_array[i];
          }
          // sort temp array
          qsort(temp, num_jobs, sizeof(job), compare_remaining_times);

          // target = job with SRT that has arrived and is not yet completed
          for (i = 0; i < num_jobs; i++)
          {
            get_remaining_run_time(temp[i].jobnum, &rem_time);
            if (qi >= temp[i].arrival_time && rem_time > 0 && temp[i].expected_run_time != rem_time)
            {
              target = temp[i].jobnum;
              printf("%d", target);
              sched_job_at_quantum(target, qi);
              ++qi;
            }
          }
        } while(target != -1);

      return 0;
}
