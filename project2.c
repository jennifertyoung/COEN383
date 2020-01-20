#include <stdio.h>
#include <stdlib.h>

struct job
{
    float arrival_time; /* random between 0.0 and 99.0 */
    float expected_run_time; /* random float between 0.1 through 10 quanta */
    int priority; /* integer 1 to 4 where 1 is highest */
    float start_time;  /*quantum when job started running for the first time. use float for easy subtraction from end time*/
    float accum_run_time; /* accum time slices */
    float end_time; /* trackes when job is complete */
};

int main()
{
    int seed = 0;
    srand(seed);
    struct job test_job = {.arrival_time = (float)(rand())/(float)(RAND_MAX) * 99.0, 
                           .expected_run_time = ((float)(rand())/(float)(RAND_MAX) * 9.9) + 0.1, 
                           .priority = (rand() % 4) + 1, 
                           .start_time = 0.0, 
                           .accum_run_time = 0.0, 
                           .end_time = 0.0};
    printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", test_job.arrival_time, test_job.expected_run_time, test_job.priority, test_job.start_time, test_job.accum_run_time, test_job.end_time);
    struct job job_array[10];
    int i = 0;
    for (i = 0; i < 10; ++i)
    {
        job_array[i].arrival_time = (float)(rand())/(float)(RAND_MAX) * 99.0; 
        job_array[i].expected_run_time = ((float)(rand())/(float)(RAND_MAX) * 9.9) + 0.1;
        job_array[i].priority = (rand() % 4) + 1;
        job_array[i].start_time = 0.0; 
        job_array[i].accum_run_time = 0.0;
        job_array[i].end_time = 0.0;
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", job_array[i].arrival_time, job_array[i].expected_run_time, job_array[i].priority, job_array[i].start_time, job_array[i].accum_run_time, job_array[i].end_time);
    }
    return 0;
}
