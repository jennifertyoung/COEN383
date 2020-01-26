#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NUM_JOBS 10

typedef struct _job
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

job job_array[NUM_JOBS];
static int theoretical_max_quantum_for_job_array;
int * highest_job_index_to_eval;

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
        printf("Job =  %.1f, %.1f, %d, %.0f, %.0f, %.0f \n", job_array[i].arrival_time, job_array[i].expected_run_time, job_array[i].priority, job_array[i].start_time, job_array[i].accum_run_time, job_array[i].end_time);
    }
    printf("After sort: \n");
    qsort(job_array, NUM_JOBS, sizeof(job), compare_jobs);
    assign_job_nums(job_array);
    return 0;
}

typedef enum _scheduling_algorithm
{
    fcfs,
    sjf,
    srt,
    rr,
    hpf_np,
    hpf_pre
} scheduling_algorithm;

typedef int(*scheduling_algorithm_function)(job * job_array);

typedef struct alg_parameters
{
    scheduling_algorithm alg;
    char * scheduling_output_file;
    scheduling_algorithm_function func;
} alg_parameters;

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

//  Theoretical max quantum for job array is the number of quanta we would need if we actually ran all the jobs generated
int precompute_job_indices()
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

    return 0;
}

int main()
{
    int seed = 10173;
    srand(seed);
    generate_and_sort_jobs();
    print_all_job_fields(job_array);
   
    compute_theoretical_max_quantum_for_job_array();

    if (precompute_job_indices() != 0)
    {
        printf("Precomputing index failed\n");
        exit(-__LINE__);
    }

    printf("Highest job index to eval: \n");
    int q = 0;
    for (q = 0; q < theoretical_max_quantum_for_job_array+1; ++q)
    {
       printf("%d ",highest_job_index_to_eval[q]);
    }
    printf("\n");
    return 0;
}
