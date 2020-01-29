#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scheduling_algorithm_api.h"
#define NUM_PRI 4

int is_job_started(int job_index);

void dump_rr_array_helper(const char * fn, int line, int * rr_job_index_array, int num_rr_entries, int priority, int qi)
{
   int ii;
   for (ii = 0; ii < num_rr_entries; ++ii)
   {
      printf("%s:%d JTY RRDBG:priority = %d ii=%d job index: %d, started= %d qi = %d\n",fn, line, priority, ii, rr_job_index_array[ii], is_job_started(rr_job_index_array[ii]) );
   }  
}

#define dump_rr_array(rji, nre, pr, qir) dump_rr_array_helper(__FUNCTION__, __LINE__, rji, nre, pr, qir)

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

static int remove_unstarted_jobs_from_rr_array(int *rr_job_index_array, int *current_rr_index, int *num_rr_entries, int quantum)
{
   int ii;
   int local_current_rr_index = *current_rr_index;
   int local_num_rr_entries = *num_rr_entries;
   for (ii = *num_rr_entries - 1; ii >= 0; --ii)
   {
       //if not able to start this job at this quantum
       if (!unfinished_job(quantum, rr_job_index_array[ii]))
       {
           int status;
           //printf("RMUNSTARTED Job to remove: %d started state: %d \n",rr_job_index_array[ii],is_job_started(rr_job_index_array[ii]));
           local_current_rr_index = ii;
           status = remove_job_at_current_rr_index(rr_job_index_array, &local_current_rr_index, &local_num_rr_entries);
           if (status != 0)
           {
               printf("%s:%d: Failed to remove.ii = %d local_num_rr_entries = %d, local_current_rr_index = %d \n", __FUNCTION__, __LINE__,  ii, local_num_rr_entries, local_current_rr_index);      
           } 
       }
   }
   if (local_num_rr_entries <= 0)
   {
      *num_rr_entries = 0;
      *current_rr_index = -1;
   }
   else
   {
      *num_rr_entries = local_num_rr_entries;
      *current_rr_index = local_current_rr_index; 
   }
   return 0;
}

int is_valid_pri(int priority)
{
   if (priority >= 1 && priority <= NUM_PRI)
   {
       return 1;
   }
   else
   {
       return 0;
   }
}

int do_hpf_pre(job * job_array, int num_jobs)
{
    int * rr_job_index_array[NUM_PRI];
    int num_rr_entries[NUM_PRI];
    int current_rr_index[NUM_PRI];
    int ii;
    for (ii = 0; ii < NUM_PRI; ++ii)
    {
        //Initialize array of pointers
        rr_job_index_array[ii] = calloc(num_jobs, sizeof(int));
        if (rr_job_index_array[ii] == NULL)
        {
            printf("Out of memory \n");
            int jj;
            for (jj = 0; jj < ii; ++jj)
            {
                if (rr_job_index_array[jj] != NULL)
                {
                    free(rr_job_index_array[jj]);
                    rr_job_index_array[jj] = NULL;
                }
            }
            return (-__LINE__);
        }
       //Initialize num_rr_entries
       num_rr_entries[ii] = 0;
       current_rr_index[ii] = -1;
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
        //Queuing all new jobs to appropriate round robin priority arrays
        int kk;
        if (qi < 5)
        {
           dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
        }
        for (kk = 0; kk < NUM_PRI; ++kk)
        {
            job * p_job = NULL;
            if (status == 0)
            {
                //add newly arrived jobs to our round robin priority arrays
                int index = lower;
                for (index = lower; index <= upper; ++index)
                {
                    p_job = &job_array[index];
                    int pri = p_job->priority-1;
                    if (unfinished_job(qi,index))
                    {
                        if (is_valid_pri(p_job->priority))
                        {
                            rr_job_index_array[pri][(num_rr_entries[pri])++] = index;
                        }
                        else
                        {
                           printf("Invalid priority=%d for job_index=%d \n", p_job->priority, index);
                        }
                    }
                }
            }
            else if (num_rr_entries[kk] == 0)
            {
                current_rr_index[kk] = -1;
            }
        }
       
        if (qi < 5)
        {
               dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
        }
        //Start dequeuing jobs 
        for (kk = 0; kk < NUM_PRI; ++kk)
        {
            //If the rr_job_index array was empty and now has entries, restart current_rr_index at 0
            if (current_rr_index[kk] == -1)
            {
                if(num_rr_entries[kk] > 0)
                {
                   current_rr_index[kk] = 0;
                }
                else
                {
                   //need to move to next queue if we have no more jobs in current queue
                   continue;
                }
            }
            //Schedule the job at the current rr index
            int return_code;
            int quantum_stop_scheduling = get_quantum_stop_scheduling_value();
            if (qi == quantum_stop_scheduling)
            {
               int err_code;
               int mm = 0;
               for (mm = 0; mm < NUM_PRI; ++mm)
               {
                   err_code = remove_unstarted_jobs_from_rr_array(rr_job_index_array[mm], &(current_rr_index[mm]), &(num_rr_entries[mm]), qi);
               
                   if (err_code != 0)
                   {
                       printf("%s:%d Error removing unstarted jobs. Error code %d \n", __FUNCTION__, __LINE__, err_code);
                   }
               }
            }
            if (current_rr_index[kk] != -1)
            {
               if (qi < 5)
               {
                  dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
               }
               return_code = sched_job_at_quantum(rr_job_index_array[kk][current_rr_index[kk]], qi);
               if (qi < 5)
               {
                  dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
               }
               if (return_code != 0)
               {
                   printf("Unexpected error scheduling job=%d, at quantum=%d \n",rr_job_index_array[kk][current_rr_index[kk]], qi);
               }
               if (unfinished_job(qi,rr_job_index_array[kk][current_rr_index[kk]]))
               {
                   if (num_rr_entries[kk] == 0)
                   {
                       printf("%s:%d Unexpected 0 RR entries. \n", __FUNCTION__, __LINE__);
                       return (-__LINE__);
                   }
                   //Move to next round robin entry. Preemptive
                   current_rr_index[kk] = (current_rr_index[kk] + 1) % num_rr_entries[kk];
                }
                else
                {
                  //Need to remove from round robin array
                   if (qi < 5)
                   {
                      dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
                   }
                  status = remove_job_at_current_rr_index(rr_job_index_array[kk], &(current_rr_index[kk]), &(num_rr_entries[kk]));
                  if (status != 0)
                  {
                      dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
                      printf("Could not remove job at current RR index =  %d for num_rr_entries = %d, for priority %d\n", current_rr_index[kk], num_rr_entries[kk], kk+1);
                      return (-__LINE__);
                  }
                  if (qi < 5)
                  {
                      dump_rr_array(rr_job_index_array[0], num_rr_entries[0], 0, qi);
                  }
                }
                if (return_code == 0)
                {
                    //Successfully scheduled a job. Move to next quantum
                    break;
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
               //All jobs are complete
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
