#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#define  NUM_JOBS 10
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

// The following is an implementation of a Queue
struct Queue
{
    int front;
    int rear;
    int size;
    int capacity;
    job* array;
};
struct Queue* init_queue()
{
    struct Queue* q = (struct Queue*) malloc(sizeof(struct Queue));
    q->capacity = NUM_JOBS;
    q->front = 0;
    q->size = 0;
    q->rear = NUM_JOBS -1;
    q->array = (job*) malloc(q->capacity * sizeof(job));
    return q;
};
int is_empty(struct Queue* q)
{
    if(q->size == 0)
        return 1;
    return 0;
}
void push(struct Queue* q, job j)
{
    q->rear = (q->rear + 1)%q->capacity;
    q->array[q->rear] = j;
    q->size++;
}
void pop(struct Queue* q)
{
    if(is_empty(q))
	    return;
    q->front = (q->front + 1) % q->capacity;
    q->size--;
}
job* front(struct Queue* q)
{
    if(is_empty(q))
    {
        return NULL;
    }
    return &q->array[q->front];
}
void print_queue(struct Queue* q)
{
    int i;
    int size = q->size;
    for(i = q->front; i < size + q->front ; i++)
    {
        printf("Job Num: %d  Job ERT: %f\n", q->array[i].jobnum, q->array[i].expected_run_time);
    }
    printf("\n");
}


// The following are functions shared across the group
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


// The following are the necessary functions to run the SJF algorithm
void sort_jobs_in_queue(struct Queue* q)
{
    int i, j;
    int size = q->size;
    job k;
    for(i = 1; i < size; i++)
    {
        k = q->array[i];
        j = i - 1;
        while(j >= 0 && q->array[j].expected_run_time > k.expected_run_time)
        {
            //printf("SWAP\n");
            q->array[j + 1] = q->array[j];
            j--;
        }
        q->array[j + 1] = k;
    }

}

// Pre:  array of the job class
// Post: 1) Process names, arrival times, run times, priority
//       2) Time chart of executed jobs
//	 3) Avg TT, WT, and RT
// 	 4) Throughput
int shortest_job_first(job * job_array)
{
    int incoming_job = 1;
    int run = 0;
    float quanta = 0;
    struct Queue* q = init_queue();

    push(q, job_array[0]);
    quanta = front(q)->arrival_time;
    // while time slice is <= 99 OR the queue is NOT empty
    while((quanta <= 99.0 || !is_empty(q)) && incoming_job-1 <= NUM_JOBS)
    {
        job* curr_proc = front(q);
        pop(q);
        printf("EXECUTING RUN #%d   Job Num: %d  Job AT: %f  Job ERT: %f Quanta: %f  Queue SZ: %d\n", run++, curr_proc->jobnum, curr_proc->arrival_time, curr_proc->expected_run_time, quanta, q->size);

        curr_proc->start_time = quanta;
        float run_time = curr_proc->expected_run_time;


        while(run_time >= 0)            // PROCESS RUNNING
        {
            if(quanta >= job_array[incoming_job].arrival_time)
            {
                push(q, job_array[incoming_job]);
                incoming_job++;
            }

            // Increment the time slice, take off time from the job's CPU time
            quanta += 1.0;
            run_time -= 1.0;
        }
        curr_proc->accum_run_time = quanta - curr_proc->start_time;
        curr_proc->end_time = quanta;

        sort_jobs_in_queue(q);
    }
    printf("\nBroke when q = %f\n\n", quanta);
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
    printf("\n",check_quantum_gaps(job_array));
    printf("\n");
    shortest_job_first(job_array);
    return 0;
}


