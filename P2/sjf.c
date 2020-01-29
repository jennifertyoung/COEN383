#include "scheduling_algorithm_api.h"

// Linearly search for the shortest run time's index
// pre: job_array with the lower and upper indices to search through
// post: an index of job_list which has the shortest run time
int find_shortest_run_time_index(job * job_list, int lower, int upper)
{
    int min_index = lower;
    float min_val = job_list[lower].expected_run_time;
    while(lower <= upper)
    {
        if(min_val > job_list[lower].expected_run_time)
        {
            min_val = job_list[lower].expected_run_time;
            min_index = lower;
        }
        lower++;
    }
    return min_index;
}

int do_sjf(job * job_array, int num_jobs)
{
    int quantum;                // Time slices
    int lower, upper;           // Indices for new job range
    int current_job_index;      // Index for job_array
    int array_head_index = 0;   // Index for head of queue
    int status;                 // Flag for first entry of function

    int max_quantum = get_theoretical_max_quantum_for_job_array();
    for(quantum = 0; quantum <= max_quantum; quantum++)
    {
        status = get_new_job_index_range(quantum, &lower, &upper);
        if(status == -1)
            current_job_index = 0;

        sched_job_at_quantum(current_job_index, quantum);

        if(job_array[current_job_index].done == 0)
            continue;

        current_job_index = find_shortest_run_time_index(job_array, ++array_head_index, upper); // return sjf index from job_array
    }
    return 0;
}
