#include <stdio.h>
#include <stdlib.h>
#define NUM_JOBS 130

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
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f %d \n", job_array[j].arrival_time, job_array[j].expected_run_time, job_array[j].priority, job_array[j].start_time, job_array[j].accum_run_time, job_array[j].end_time, job_array[j].jobnum); 
    }
}

/*Returns 1 if gap of 2 quanta exists in job array*/
int check_quantum_gaps(job * job_array)
{
    int i = 0;
    for (i = 0; i < NUM_JOBS - 1; ++i)
    {
        //printf("i = %d \n",i);
        //printf("%f \n", job_array[i].arrival_time);
        float max_interval = job_array[i].arrival_time + job_array[i].expected_run_time;
        if (job_array[i+1].arrival_time - max_interval > 2.0)
        {
            printf("End time of job %d = %f", job_array[i].jobnum, max_interval);
            printf(" Arrival time of job %d  = %f", job_array[i+1].jobnum, job_array[i+1].arrival_time);
            printf(" CPU Idle = %f \n", job_array[i+1].arrival_time - max_interval);
            return 1;
        }
    }
    return 0;
}

int main()
{
    int seed = 10;
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
    int j = 0;
    for (j = 0; j < NUM_JOBS; ++j)
    {
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", job_array[j].arrival_time, job_array[j].expected_run_time, job_array[j].priority, job_array[j].start_time, job_array[j].accum_run_time, job_array[j].end_time);       
    }
    assign_job_nums(job_array);
    print_all_job_fields(job_array);
    printf("%d",check_quantum_gaps(job_array));
    return 0;
}
