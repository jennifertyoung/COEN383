#include <stdio.h>
#include <stdlib.h>
#define NUM_JOBS 20

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
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f, %d \n", job_array[j].arrival_time, job_array[j].expected_run_time, job_array[j].priority, job_array[j].start_time, job_array[j].accum_run_time, job_array[j].end_time, job_array[j].jobnum); 
    }
}

int check_quantum_gaps(job * job_array)
{
    float left = job_array[0].arrival_time;
    float right = job_array[0].arrival_time + job_array[0].expected_run_time;
    int i;
    for (i = 0; i < NUM_JOBS; ++i)
    {
       if (job_array[i].arrival_time > left && (job_array[i].arrival_time + job_array[i].expected_run_time) < right)
       {
           printf ("%d job is completely contained \n",i);
           //job is completely contained
           left = job_array[i].arrival_time;
       }
       else if (job_array[i].arrival_time > left && (job_array[i].arrival_time) > right)
       {
           printf("%d we have a gap \n", i);
           if (job_array[i].arrival_time - right > 2.0)
           {
               printf("Gap = %f \n",job_array[i].arrival_time - right);
               printf("Gap too large. Regenerate Jobs. \n");
               return 1;
           }
       }
       else if (job_array[i].arrival_time > left && job_array[i].arrival_time + job_array[i].expected_run_time > right)
       {
           printf("%d job finishes after old right. update right \n", i);
           right = job_array[i].arrival_time;
       }
    }
    return 0;
}

int main()
{
    int seed = 10173;
    srand(seed);
    job job_array[NUM_JOBS];
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
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", job_array[i].arrival_time, job_array[i].expected_run_time, job_array[i].priority, job_array[i].start_time, job_array[i].accum_run_time, job_array[i].end_time);
    }
    printf("After sort: \n");
    qsort(job_array, NUM_JOBS, sizeof(job), compare_jobs);
    
    assign_job_nums(job_array);
    print_all_job_fields(job_array);
    printf("%d",check_quantum_gaps(job_array));
    return 0;
}
