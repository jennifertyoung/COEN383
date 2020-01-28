#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scheduling_algorithm_api.h"

static int remove_job_at_current_rr_index(int *rr_job_index_array, int *current_rr_index, int *num_rr_entries)
{
    if (*num_rr_entries <= 0)
    {
        printf("Attempting to remove entry from empty round robin job index array at rr index %d, num_rr_entries = %d \n", *current_rr_index, *num_rr_entries);
        *num_rr_entries = 0;
        return (-__LINE__);
    }
    else if (*num_rr_entries == 1)
    {
        //reset the current rr index because round robin array is empty
        *current_rr_index = -1;
        *num_rr_entries = 0;
    }
    else
    {
        int ri = 0;
        for(ri = *current_rr_index; ri < (*num_rr_entries - 1); ++ri)
        {
            rr_job_index_array[ri] = rr_job_index_array[ri+1];
        }
        
        *num_rr_entries = *num_rr_entries - 1;
        if (*current_rr_index >= *num_rr_entries)
        {
           *current_rr_index = 0;
        }
        else
        {
           *current_rr_index = (*current_rr_index + 1) % *num_rr_entries;
        }
    }
    return 0;
}

int remove_unstarted_jobs_from_rr_array(int *rr_job_index_array, int *current_rr_index, int *num_rr_entries, int quantum)
{
   int ii;
   int local_current_rr_index = *current_rr_index;
   int local_num_rr_entries = *num_rr_entries;
   for (ii = *num_rr_entries - 1; ii >= 0; --ii)
   {
       if (!unfinished_job(quantum, rr_job_index_array[ii]))
       {
           int status;
           status = remove_job_at_current_rr_index(rr_job_index_array, &local_current_rr_index, &local_num_rr_entries);
           if (status != 0)
           {
               printf("%s:%d: Failed to remove.ii = %d local_num_rr_entries = %d, local_current_rr_index = %d \n", __FUNCTION__, __LINE__,  ii, local_num_rr_entries, local_current_rr_index);      
           } 
       }
   }
   if (local_num_rr_entries <= 0)
   {
      *num_rr_entries = local_num_rr_entries;
      *current_rr_index = -1;
   }
   else
   {
      *num_rr_entries = local_num_rr_entries;
      *current_rr_index = local_current_rr_index; 
   }
   return 0;
}

int do_rr(job * job_array, int num_jobs)
{
    int * rr_job_index_array;
    rr_job_index_array = calloc(num_jobs, sizeof(int));
    int num_rr_entries = 0;
    int current_rr_index = -1;
    if (rr_job_index_array == NULL)
    {
        return (-__LINE__);
    }
    int qi = 0;
    int max_quantum = get_theoretical_max_quantum_for_job_array();
    printf("Max Quantum: %d \n", max_quantum);
    for (qi = 0; qi <= max_quantum; ++qi)
    {
        int lower;
        int upper;
        int status;
        status = get_new_job_index_range(qi, &lower, &upper);
        if (status != 0 && num_rr_entries == 0)
        {
            current_rr_index = -1;
            continue;
        }
        else if (status == 0)
        {
//            printf("%s:%d qi = %d, lower = %d, upper = %d \n", __FUNCTION__, __LINE__, qi, lower, upper);
//            printf("%s:%d current_rr_index = %d, num_rr_entries = %d \n", __FUNCTION__, __LINE__, current_rr_index, num_rr_entries);
            //add newly arrived jobs to our round robin array
            int index = lower;
            for (index = lower; index <= upper; ++index)
            {
                if (unfinished_job(qi,index))
                {
                    rr_job_index_array[num_rr_entries++] = index;
                }
            }
            //printf("%s:%d qi = %d, lower = %d, upper = %d \n", __FUNCTION__, __LINE__, qi, lower, upper);
            //printf("%s:%d current_rr_index = %d, num_rr_entries = %d \n", __FUNCTION__, __LINE__, current_rr_index, num_rr_entries);
        }
        //If the rr_job_index array was empty and now has entries, restart current_rr_index at 0
        if (current_rr_index == -1)
        {
            current_rr_index = 0;
        }
        //Schedule the job at the current rr index
        int return_code;
        int quantum_stop_scheduling = get_quantum_stop_scheduling_value();
        if (qi == quantum_stop_scheduling)
        {
           int err_code;
           err_code = remove_unstarted_jobs_from_rr_array(rr_job_index_array, &current_rr_index, &num_rr_entries, qi);
           if (err_code != 0)
           {
               printf("%s:%d Error removing unstarted jobs. Error code %d \n", __FUNCTION__, __LINE__, err_code);
           }
        }
        if (current_rr_index != -1)
        {
           return_code = sched_job_at_quantum(rr_job_index_array[current_rr_index], qi);
           if (unfinished_job(qi,rr_job_index_array[current_rr_index]))
           {
    //           printf("%s:%d qi = %d \n", __FUNCTION__, __LINE__, qi);
               if (num_rr_entries == 0)
               {
                   printf("Unexpected 0 RR entries. \n");
    //               printf("%s:%d qi = %d\n", __FUNCTION__, __LINE__, qi);
                   return (-__LINE__);
               }
               //Move to next round robin entry. Preemptive
               current_rr_index = (current_rr_index + 1) % num_rr_entries;
    //           printf("%s:%d current_rr_index = %d \n", __FUNCTION__, __LINE__, current_rr_index);
            }
            else
            {
              //Need to remove from round robin array
    //          printf("%s:%d current_rr_index = %d num_rr_entries = %d \n", __FUNCTION__, __LINE__, current_rr_index, num_rr_entries);
              status = remove_job_at_current_rr_index(rr_job_index_array, &current_rr_index, &num_rr_entries);
    //          printf("%s:%d After removal current_rr_index = %d num_rr_entries = %d \n", __FUNCTION__, __LINE__, current_rr_index, num_rr_entries);
              if (status != 0)
              {
                  printf("Could not remove job at current RR index =  %d for num_rr_entries = %d\n", current_rr_index, num_rr_entries);
                  return (-__LINE__);
              }
            }
        }

        if (qi >= get_quantum_stop_scheduling_value())
        {
           int lower_s;
           int upper_s;
           status = get_unfinished_job_index_range(qi, &lower_s, &upper_s);
           if (status != 0)
           {
               if (status > 0)
               {
                   break;
               }
               else
               {
                   return status;
               }
           }
           else
           {
               //There are started jobs that need to be finished.
           }    
        }
    }
    return 0;
}
