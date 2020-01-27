#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NUM_JOBS 10
#include "scheduling_algorithm_api.h"

job job_array[NUM_JOBS];
static int theoretical_max_quantum_for_job_array;
int * highest_job_index_to_eval;
int * all_done;
int quantum_stop_scheduling = 100;

static FILE * current_quanta_chart_fp = NULL;

//Get quantum after which we stop scheduling new jobs
int get_quantum_stop_scheduling_value()
{
    return quantum_stop_scheduling;
}

int compare_jobs(const void * a, const void * b)
{
    job *job_a = (job *) a;
    job *job_b = (job *) b;
    if (job_a->arrival_time < job_b->arrival_time)
        return -1;
    else if (job_a->arrival_time > job_b->arrival_time)
        return 1;
    else
        return 0;
}

void assign_job_nums(job * job_array)
{ 
    int k = 0;
    for (k = 0; k < NUM_JOBS; ++k)
    {
        job_array[k].jobnum = k;
    }
}

void print_all_job_fields(job * job_array)
{
    int j = 0;
    for (j = 0; j < NUM_JOBS; ++j)
    {
        printf("Job =  %f, %f, %d, %.0f, %.0f, %.0f, %d \n", job_array[j].arrival_time, job_array[j].expected_run_time, job_array[j].priority, job_array[j].start_time, job_array[j].accum_run_time, job_array[j].end_time, job_array[j].jobnum); 
    }
}

int generate_and_sort_jobs()
{
    int i = 0;
    printf("Before sort: \n");
    for (i = 0; i < NUM_JOBS; ++i)
    {
        job_array[i].arrival_time = (float)(rand())/(float)(RAND_MAX) * 99.0; 
        job_array[i].expected_run_time = ((float)(rand())/(float)(RAND_MAX) * 9.9) + 0.1;
        job_array[i].priority = (rand() % 4) + 1;
        job_array[i].start_time = 0.0; 
        job_array[i].accum_run_time = 0.0;
        job_array[i].end_time = 0.0;
        job_array[i].jobnum = 0; //this will be overwritten after sort
        job_array[i].done = 0;
        job_array[i].started = 0;
        job_array[i].sched_allowed = 1;
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", job_array[i].arrival_time, job_array[i].expected_run_time, job_array[i].priority, job_array[i].start_time, job_array[i].accum_run_time, job_array[i].end_time);
    }
    printf("After sort: \n");
    qsort(job_array, NUM_JOBS, sizeof(job), compare_jobs);
    assign_job_nums(job_array);
    return 0;
}

/*Define Scheduling Algorithm API*/

//Returns 1 if job unfinished, 0 otherwise
int unfinished_job(int quantum, int ji)
{
//   printf("%s:%d quantum = %d, job_index %d \n", __FUNCTION__, __LINE__, quantum, ji);
   if (quantum < quantum_stop_scheduling)
   {
//       printf("%s:%d quantum = %d, job_index %d \n", __FUNCTION__, __LINE__, quantum, ji);
//       printf("Job_Array.done = %d, Sched_Allowed = %d \n", job_array[ji].done, job_array[ji].sched_allowed);
       return (job_array[ji].done == 0);
   }
   else
   {
//       printf("%s:%d quantum = %d, job_index %d \n", __FUNCTION__, __LINE__, quantum, ji);
//       printf("Job_Array.done = %d, Sched_Allowed = %d \n", job_array[ji].done, job_array[ji].sched_allowed);
       return ( (job_array[ji].done == 0) && (job_array[ji].sched_allowed == 1) );
   }
}

int get_unfinished_job_index_range(int quantum, int *lower, int *upper)
{
   if (quantum <= 0 || quantum > theoretical_max_quantum_for_job_array)
   {
       //Invalid quantum given. Return negative number.
       return (-__LINE__);
   }
   else
   {
       if (highest_job_index_to_eval[quantum] < 0)
       {
           //No unfinished jobs. Return positive number.
           all_done[quantum] = 1;
           return (__LINE__);
       }
       else
       {
           int lower_new;
           int upper_new;
           int ji = 0;
           int lower_found = 0;
           for (ji = 0; ji <= highest_job_index_to_eval[quantum]; ++ji)
           {
              if (unfinished_job(quantum, ji))
              {
                  if (lower_found == 0)
                  {
                     lower_new = ji;
                     lower_found = 1;
                  }
                  upper_new = ji;
              }
           }
           if (lower_found == 0)
           {
               all_done[quantum] = 1;
               return (__LINE__);
           }
           else
           {
               *lower = lower_new;
               *upper = upper_new;
           }
       }
   }
   return 0;
}

//Only valid at the start of the quantum before any new jobs have been scheduled
int get_new_job_index_range(int quantum, int *lower_new, int *upper_new)
{
   if ( (highest_job_index_to_eval[quantum-1]+1) > highest_job_index_to_eval[quantum])
   {
       //No unfinished jobs. Return positive number.
       return (__LINE__);
   }
   else
   {
       *lower_new = highest_job_index_to_eval[quantum-1] + 1;
       *upper_new = highest_job_index_to_eval[quantum];
   }
   return 0;
}

int is_job_done(int job_index)
{
   if (job_index < 0 || job_index >= NUM_JOBS)
   {
       return 0;
   }
   else
   {
       return job_array[job_index].done;
   }
}

int is_job_started(int job_index)
{
   if (job_index < 0 || job_index >= NUM_JOBS)
   {
       return 0;
   }
   else
   {
       return job_array[job_index].started;
   }
}

int get_remaining_run_time(int job_index, float *rem_time)
{
   if (job_index < 0 || job_index >= NUM_JOBS)
   {
       printf("Invalid job index to get remaining run time %d", job_index);
       return (-__LINE__);
   }
   else
   {
       *rem_time = job_array[job_index].expected_run_time - job_array[job_index].accum_run_time;
   }
   return 0;
}

int are_all_jobs_done(int quantum)
{
    return 0;
}

static void stop_new_jobs_scheduling(void)
{
   int ji;
   for (ji = 0; ji < NUM_JOBS; ++ji)
   {
       if ( (job_array[ji].done == 0) && (job_array[ji].started == 0))
       {
           job_array[ji].sched_allowed = 0;
       }
   }
}

int update_quanta_chart(job_index)
{
   if (current_quanta_chart_fp == NULL)
   {
       return (-__LINE__);
   }
   else
   {
       if (job_index >= 0 && job_index < NUM_JOBS)
       {
          fprintf(current_quanta_chart_fp,"%d, ", job_array[job_index].jobnum);
       }
       else
       {
          return (-__LINE__);
       }
   }
   return 0;
}

static int scheduling_stop_called = 0;
int sched_job_at_quantum(int job_index, int quantum)
{
//   printf("Job index %d, Quantum %d \n", job_index, quantum);
   if (job_index < 0 || job_index >= NUM_JOBS)
   {
       printf("Invalid job index to schedule %d \n", job_index);
       return (-__LINE__);
   }
   else if (quantum < 0 || quantum > theoretical_max_quantum_for_job_array)
   {
       printf("Invalid quantum to schedule %d \n", quantum);
       return (-__LINE__);
   }
   else
   {
       if ( (scheduling_stop_called == 0) && (quantum >= quantum_stop_scheduling) )
       {
           stop_new_jobs_scheduling();
           scheduling_stop_called = 1;
       }
       if (unfinished_job(quantum, job_index))
       {
           if (job_array[job_index].started == 0)
           {
               //start the job
               //We already prevented starting new jobs after quantum_stop_scheduling (100)
               job_array[job_index].started = 1;
               job_array[job_index].start_time = quantum;
           }
           
           float rem_time;
           int status = get_remaining_run_time(job_index, &rem_time);
           int quanta_update_status = 0;
           if (status == 0)
           {
               //write out to quanta chart since we are scheduling this job
               quanta_update_status = update_quanta_chart(job_index);
               if (quanta_update_status != 0)
               {
                  printf("Failed up update quanta chart %d for job index %d at quantum %d \n", quanta_update_status, job_index, quantum);
               }
               if (rem_time <= 1.0)
               {
                  //done
                  job_array[job_index].accum_run_time += rem_time;
                  job_array[job_index].end_time = (float) quantum + rem_time;
                  job_array[job_index].done = 1;
               }
               else
               {
                  //job not done
                  job_array[job_index].accum_run_time += 1.0;
               }
           }
           else
           {
               printf("Impossible case: %d\n", status);
               return (-__LINE__);
           }
       }
   }
   return 0;
}

typedef enum _scheduling_algorithm_e
{
    fcfs,
    sjf,
    srt,
    rr,
    hpf_np,
    hpf_pre
} scheduling_algorithm_e;


int do_fcfs(job * job_array, int num_jobs)
{
    return 0;
}

int do_sjf(job * job_array, int num_jobs)
{
    return 0;
}

int do_srt(job * job_array, int num_jobs)
{
    return 0;
}

extern int do_rr(job * job_array, int num_jobs);

int do_hpf_np(job * job_array, int num_jobs)
{
    return 0;
}

int do_hpf_pre(job * job_array, int num_jobs)
{
    return 0;
}

typedef int(*scheduling_algorithm_function)(job * job_array, int num_jobs);

typedef struct alg_parameters
{
    scheduling_algorithm_e alg;
    char * scheduling_output_file;
    scheduling_algorithm_function func;
} alg_parameters;

alg_parameters scheduling_algorithm[] =
{
#if 0
    {fcfs, "./fcfs_sched_out", do_fcfs},
    {sjf, "./sjf_sched_out", do_sjf},
    {srt, "./srt_sched_out", do_srt},
    {rr, "./rr_sched_out", do_rr},
    {hpf_np, "./hpf_np_sched_out", do_hpf_np},
    {hpf_pre, "./hpf_pre_sched_out", do_hpf_pre}
#else
    {rr, "./rr_sched_out", do_rr}
#endif
};

int num_alg_defined = sizeof(scheduling_algorithm)/sizeof(scheduling_algorithm[0]);

//returns 1 if CPU is idle for 2 or more quanta, 0 otherwise.
/*int check_quantum_gaps(job * job_array)
{
    int left_job_index = 0;
    int right_job_index = 1;
    int i;
    for (i = 1; i < NUM_JOBS; ++i)
    {
        right_job_index = i;
        float left_arrival_time = job_array[left_job_index].arrival_time;
        float left_finish_time = job_array[left_job_index].arrival_time + job_array[left_job_index].expected_run_time;
        float right_arrival_time = job_array[i].arrival_time;
        float right_finish_time = job_array[i].arrival_time + job_array[i].expected_run_time;
        if (right_finish_time <= left_finish_time)
        {
            //right job is completely contained
            continue;
        }
        else if (right_arrival_time <= left_finish_time)
        {
            //partial overlap
            left_job_index = i;
            continue;
        }
        else
        {
            //we have a gap. Adding 0.5 and casting to int is like the ceiling. Right arrival time won't be able to start until next quantum.
            //Need to take ceiling of left finish time because CPU was not idle during part of this quantum
            int gap_quanta = (int) ceil(right_arrival_time) - (int) ceil(left_finish_time);
            printf("Left Job Index = %d, Right Job Index = %d \n", left_job_index, right_job_index);
            printf("Gap = %d \n",gap_quanta);
            if (gap_quanta >= 2)
            {
                printf("Generate more jobs!\n");
                return 1;
            }
            //gap small enough
            left_job_index = i;
        }
    }
    return 0;
}
*/
int compute_theoretical_max_quantum_for_job_array()
{
    theoretical_max_quantum_for_job_array = (int) ceil(job_array[0].arrival_time);
    int left_job_index = 0;
    int right_job_index = 1;
    int i;
    for (i = 1; i < NUM_JOBS; ++i)
    {
        right_job_index = i;
        float left_arrival_time = job_array[left_job_index].arrival_time;
        float left_finish_time = job_array[left_job_index].arrival_time + job_array[left_job_index].expected_run_time;
        float right_arrival_time = job_array[i].arrival_time;
        float right_finish_time = job_array[i].arrival_time + job_array[i].expected_run_time;
        int partial_overlap = right_arrival_time <= left_finish_time;
        int completely_contained = right_finish_time <= left_finish_time;
        if (completely_contained || partial_overlap)
        {
            //any overlap
            theoretical_max_quantum_for_job_array += (int) ceil(job_array[i].expected_run_time);
            if (partial_overlap)
            {
                left_job_index = i;
            }
        }
        else
        {
            //we have a gap
            int gap_quanta = (int) ceil(right_arrival_time) - (int) ceil(left_finish_time);
            theoretical_max_quantum_for_job_array += gap_quanta;
            theoretical_max_quantum_for_job_array += (int) ceil(job_array[i].expected_run_time);
            left_job_index = i;
        }
    }
    return theoretical_max_quantum_for_job_array;
}

static int theoretical_max_initialized = 0;
int get_theoretical_max_quantum_for_job_array()
{
    if (theoretical_max_initialized == 0) 
    { 
       compute_theoretical_max_quantum_for_job_array();
       theoretical_max_initialized = 1;
    }
    return theoretical_max_quantum_for_job_array; 
}

//  Theoretical max quantum for job array is the number of quanta we would need if we actually ran all the jobs generated
int allocate_quanta_helper_arrays()
{
    highest_job_index_to_eval = calloc(theoretical_max_quantum_for_job_array+1,sizeof(int));
    if (highest_job_index_to_eval == NULL)
    {
        printf("Unable to allocate memory for highest_job_index_to_eval \n");
        return (-__LINE__);
    }
    int m = 0;
    int right_quantum = theoretical_max_quantum_for_job_array + 1;
    int left_quantum; //index for highest_job_index_to_eval 
    for (m = NUM_JOBS - 1; m >= 0; --m)
    {
       left_quantum = (int) ceil(job_array[m].arrival_time);
       int qi = 0;
       for (qi = left_quantum; qi < right_quantum; ++qi)
       {
          highest_job_index_to_eval[qi] = m;
       }
       right_quantum = left_quantum; 
    }
    int p = 0;
    for (p = 0; p < (int) ceil(job_array[0].arrival_time); ++p)
    {
       highest_job_index_to_eval[p] = -1;
    }

    all_done = calloc(theoretical_max_quantum_for_job_array+1,sizeof(int));
    if (all_done == NULL)
    {
        printf("Unable to allocate memory for all_done \n");
        return (-__LINE__);
    }
    return 0;
}

int testing()
{
    int lower;
    int upper;
    int return_val;
    int quantum_range;
    for (quantum_range = 0; quantum_range < 110; ++quantum_range)
    {
       return_val = get_unfinished_job_index_range(quantum_range, &lower, &upper);
       if (return_val == 0)
       {
          printf("Quantum: %d Lower: %d Upper %d \n", quantum_range, lower, upper);
       }
       else
       {
          printf("Quantum: %d Status: %d \n", quantum_range, return_val);
       }
    }

    for (quantum_range = 0; quantum_range < 100; ++quantum_range)
    {
       return_val = get_new_job_index_range(quantum_range, &lower, &upper);
       if (return_val == 0)
       {
          printf("Quantum: %d Lower New: %d Upper New: %d \n", quantum_range, lower, upper);
       }
       else
       {
          printf("Quantum: %d Status: %d \n", quantum_range, return_val);
       }
    }

    float rem_time;
    int jobi = 0;
    for (jobi = 0; jobi < NUM_JOBS; ++jobi)
    {
        int status;
        status = get_remaining_run_time(jobi, &rem_time);
        if (status != 0)
        {
            printf("Error Job Index %d\n", jobi);
        }
        else
        {
            printf("JOB %d Remaining run time %f\n", jobi, rem_time);
        }
    }
    printf ("Theoretical max: %d \n", theoretical_max_quantum_for_job_array);
    return 0;
}



int create_quanta_chart(int run_number, alg_parameters *alg_ptr)
{
    char file_name[80] = {'\0'};
    snprintf(file_name, sizeof(file_name), "%s_run%d.txt", alg_ptr->scheduling_output_file, run_number);
    printf("file name %d = %s \n", alg_ptr->alg, file_name);
    current_quanta_chart_fp = fopen(file_name, "w");
    if (current_quanta_chart_fp == NULL)
    {
        printf("Unable to create file for %s\n", file_name);
        return (-__LINE__);
    }
    return 0;
}

int display_job_stats(scheduling_algorithm_e alg, int run)
{
    printf("Displaying stats for algorithm %d run %d \n", alg, run);
    int job_index;
    float turnaround_time = 0.0;
    float waiting_time = 0.0;
    float response_time = 0.0;
    float avg_turnaround_time = 0.0;
    float avg_waiting_time = 0.0;
    float avg_response_time = 0.0;
    int num_done_jobs = 0;
    for (job_index = 0; job_index < NUM_JOBS; ++job_index)
    {
        job * p_job = &job_array[job_index];
        if (p_job->done)
        {
           turnaround_time = p_job->end_time - p_job->arrival_time;
           response_time = p_job->start_time - p_job->arrival_time;
           waiting_time = turnaround_time - p_job->expected_run_time;
           avg_turnaround_time += p_job->end_time - p_job->arrival_time;
           avg_response_time += p_job->start_time - p_job->arrival_time;
           avg_waiting_time += turnaround_time - p_job->expected_run_time;
           ++num_done_jobs;
        }
    }
    if (num_done_jobs != 0)
    {
        avg_turnaround_time /= (float) num_done_jobs;
        avg_response_time /= (float) num_done_jobs;
        avg_waiting_time /= (float) num_done_jobs;
    }
    printf("Average Turnaround Time %f \n", avg_turnaround_time);
    printf("Average Response Time %f \n", avg_response_time);
    printf("Average Waiting Time %f \n", avg_waiting_time);
    return 0;
}    

int cleanup_simulation_run()
{
    scheduling_stop_called = 0;
    if (current_quanta_chart_fp != NULL)
    {
        fclose(current_quanta_chart_fp);
        current_quanta_chart_fp = NULL;
    }
    return 0;
}

int main()
{
    int run = 0;
    int seed = 10173;
    srand(seed);
    generate_and_sort_jobs();
    print_all_job_fields(job_array);
   
    get_theoretical_max_quantum_for_job_array();

    if (allocate_quanta_helper_arrays() != 0)
    {
        printf("Precomputing index failed\n");
        exit(-__LINE__);
    }

    //check highest job index to eval array
/*    printf("Highest job index to eval: \n");
    int q = 0;
    for (q = 0; q < theoretical_max_quantum_for_job_array+1; ++q)
    {
       printf("%d ",highest_job_index_to_eval[q]);
    }
    printf("\n");
*/
    int status = 0;
    int alg_index = 0;
    for (alg_index = 0; alg_index < num_alg_defined; ++alg_index)
    {
       alg_parameters * alg_ptr = &scheduling_algorithm[alg_index];
       status = create_quanta_chart(run, alg_ptr);
       status = alg_ptr->func(job_array, NUM_JOBS);
       if (status != 0)
       {
           printf("Failed to complete run: %d algorithm: %d", run, alg_ptr->alg); 
       }
       display_job_stats(alg_ptr->alg, run);
       status = cleanup_simulation_run();
       if (status != 0)
       {
           printf("Failed to cleanup: %d algorithm: %d", run, alg_ptr->alg); 
       }
    }

    return 0;

}
