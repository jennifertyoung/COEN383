#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NUM_JOBS 90

typedef struct
{
    float arrival_time; /* random between 0.0 and 99.0 */
    float expected_run_time; /* random float between 0.1 through 10 quanta */
    int priority; /* integer 1 to 4 where 1 is highest */
    float start_time;  /*quantum when job started running for the first time. use float for easy subtraction from end time*/
    float accum_run_time; /* accum time slices */
    float end_time; /* tracks when job is complete */
    //char name[10];
    int jobnum;
} job;

int compare_jobs(const void * a, const void * b)
{
    job *job_a = (job *) a;
    job *job_b = (job *) b;
    return(job_a->arrival_time - job_b->arrival_time);
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
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f, %d \n", job_array[j].arrival_time, job_array[j].expected_run_time, job_array[j].priority, job_array[j].start_time, job_array[j].accum_run_time, job_array[j].end_time, job_array[j].jobnum); 
    }
}

//returns 1 if CPU is idle for 2 or more quanta, 0 otherwise.
int check_quantum_gaps(job * job_array)
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

void hfpf_schedule(job *);

int main()
{
    int seed = 10173;
    srand(seed);
    job job_array[NUM_JOBS];
    int i = 0;
    for (i = 0; i < NUM_JOBS; ++i)
    {
        job_array[i].arrival_time = (float)(rand())/(float)(RAND_MAX) * 99.0; 
        job_array[i].expected_run_time = ((float)(rand())/(float)(RAND_MAX) * 9.9) + 0.1;
        job_array[i].priority = (rand() % 4) + 1;
        job_array[i].start_time = 0.0; 
        job_array[i].accum_run_time = 0.0;
        job_array[i].end_time = 0.0;
    }
    qsort(job_array, NUM_JOBS, sizeof(job), compare_jobs);
    
    assign_job_nums(job_array);
    check_quantum_gaps(job_array);

    hpfp_schedule(job_array);

    return 0;
}
