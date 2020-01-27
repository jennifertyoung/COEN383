#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_JOBS 90

// Infrastructure Functions
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

// Array implementation of a queue
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
    q->rear = (q->rear + 1) % q->capacity;
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
        printf("Job Num: %d  Job ERT: %f  Index: %d\n", q->array[i].jobnum, q->array[i].expected_run_time, i);
    }
    printf("\n");
}

// Metrics
float get_average_turnaround_time(job *list, int n)
{
    float turnaround_val;
    int i;
    for(i = 0; i < n; i++)
        turnaround_val = (list[i].end_time - list[i].arrival_time);

    turnaround_val /= n;
    return turnaround_val;
}
float get_average_wait_time(job *list, int n)
{
    float wait_val;
    int i;
    // Turnaround = End - Arrival
    for(i = 0; i < n; i++)
        wait_val += (list[i].end_time - list[i].arrival_time) - list[i].expected_run_time;
    wait_val /= n;
    return wait_val;
}
float get_average_response_time(job *list, int n)
{
    float response_val;
    int i;
    for(i = 0; i < n; i++)
        response_val += list[i].start_time - list[i].arrival_time;
    response_val /= n;
    return response_val;
}
float get_throughput(float quanta, int n)
{
    return quanta/n;
}
void print_process_metrics(job *list, int n)
{
    printf("Average Turnaround Time: %f\n", get_average_turnaround_time(list, n));
    printf("Average Wait Time: %f\n", get_average_wait_time(list, n));
    printf("Average Response Time: %f\n", get_average_response_time(list, n));
}
void print_algorithm_metrics(float quanta, int n)
{
    printf("Throughput: %f\n",get_throughput(quanta, n));
}
void print_time_chart(job *list, int n)
{
    printf("\nProcess Time Chart: ");
    int i;
    for(i = 0; i < n; i++)
    {
        printf("%d ", list[i].jobnum);
    }
    printf("\n");
}

// Insertion sort specialized for the queue
void sort_jobs_in_queue(struct Queue* q)
{
    int i, j;
    int size = q->size + q->front;
    job k;
    for(i = q->front + 1; i < size; i++)
    {
        k = q->array[i];
        j = i - 1;
        while(j >= q->front && q->array[j].expected_run_time > k.expected_run_time)
        {
            q->array[j + 1] = q->array[j];
            j--;
        }
        q->array[j + 1] = k;
    }
}
void shortest_job_first(job * job_array)
{
    float quanta = 0;               // Time slices
    int incoming_job = 1;           // Index jobs added to queue (not necessarily given CPU time)
    int index = 0;                  // Index jobs given to CPU
    job jobs_executed[NUM_JOBS];    // Jobs given to CPU
    struct Queue* q = init_queue(); // Queue for processes

    push(q, job_array[0]);
    quanta = front(q)->arrival_time;
    while((quanta <= 99.0 || !is_empty(q)) && incoming_job <= NUM_JOBS)
    {
        // Assign job at the front of the queue, and pop it
        job* curr_proc = front(q);
        pop(q);

        printf("Job Num: %d  Job Arrival Time: %f  Job ERT: %f Priority: %d\n", curr_proc->jobnum, curr_proc->arrival_time, curr_proc->expected_run_time, curr_proc->priority);

        float start_time = quanta;
        float run_time = curr_proc->expected_run_time;
        // PROCESS RUNNING
        while(run_time >= 0)
        {
            // IF the arrival time has been passed, THEN push it to the queue
            if(quanta >= job_array[incoming_job].arrival_time)
            {
                push(q, job_array[incoming_job]);
                incoming_job++;
            }
            quanta += 1.0;
            run_time -= 1.0;
        }

        // Assign necessary metrics to the current process
        curr_proc->start_time = start_time;
        curr_proc->accum_run_time = (quanta - curr_proc->start_time);
        curr_proc->end_time = quanta;
        jobs_executed[index++] = *curr_proc;

        sort_jobs_in_queue(q);
    }

    // Provide metrics to user
    print_time_chart(jobs_executed, index);
    print_process_metrics(jobs_executed, index);
    print_algorithm_metrics(quanta, index);
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


