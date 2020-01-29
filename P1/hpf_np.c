#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scheduling_algorithm_api.h"

int do_hpf_np(job * job_array, int num_jobs) {
    int * hpf_job_index_queues[4];
    hpf_job_index_queues[0] = calloc(num_jobs, sizeof(int));
    hpf_job_index_queues[1] = calloc(num_jobs, sizeof(int));
    hpf_job_index_queues[2] = calloc(num_jobs, sizeof(int));
    hpf_job_index_queues[3] = calloc(num_jobs, sizeof(int));

    int num_hpf_entries_started[4] = {0,0,0,0};
    int num_hpf_entries_done[4] = {0,0,0,0};
    int num_hpf_entries[4] = {0,0,0,0};
    int num_hpf_entries_total = 0;
    int current_hpf_index = -1;
    int target_entry = -1;
    int target_prio = 0;

    int i;
    int j;    
    int qi = 0;
    int max_quantum = get_quantum_stop_scheduling_value();
    printf("Max Quantum: %d \n", max_quantum);

    float remaining_run_time;

    for (i = 0; i < 4; ++i) {
        if (hpf_job_index_queues[i] == NULL) {
            return (-__LINE__);
        }
    }

    for (qi = 0; qi < max_quantum; ++qi) {
        int lower;
        int upper;
        int status;
        status = get_new_job_index_range(qi, &lower, &upper);
        if (status == 0) {
            // Add newly arrived jobs to the appropriate priority queue
            int index = lower;
            for (index = lower; index <= upper; ++index) {
                if (unfinished_job(qi,index)) {
                    target_prio = 0;
                    target_prio = job_array[index].priority - 1;
                    hpf_job_index_queues[target_prio][num_hpf_entries[target_prio]] = index;
                    ++num_hpf_entries[target_prio];
                    ++num_hpf_entries_total;
                }
            }
        }
        
        if (target_entry == -1) {
            // If nothing is already running
            // Find the highest priority job in the queues
            target_prio = 0;
            while (target_prio < 3 && num_hpf_entries_done[target_prio] >= num_hpf_entries[target_prio]) {
                ++target_prio;
            }       
            if (num_hpf_entries_done[target_prio] < num_hpf_entries[target_prio]) { 
                // something is in the queue
                target_entry = hpf_job_index_queues[target_prio][num_hpf_entries_done[target_prio]];
            }
        }
        
        if (target_entry != -1) {
            // run the target entry
            sched_job_at_quantum(target_entry, qi);
            // job is first run this quanta
            if (job_array[target_entry].start_time == qi) {
                ++num_hpf_entries_started[target_prio];
            }
            if (!unfinished_job(qi,target_entry)) {
                // If job is finished, track its completion
                ++num_hpf_entries_done[job_array[target_entry].priority - 1];
                target_entry = -1;   
            }
        }
    }
    
    if (target_entry != 1) {
        // There is a job to finish
        get_remaining_run_time(target_entry, &remaining_run_time);
        for (i = 0; j < ceil(remaining_run_time); ++j) {
            ++qi;
            sched_job_at_quantum(target_entry, qi);        
        } 
        ++num_hpf_entries_done[job_array[target_entry].priority -1];
    }

    
    /****************************/
    /*      Output Report       */
    /****************************/

    float metrics_array[4][4];
    //run per queue analysis
    printf("\nper queue analysis:\n");
    
    printf("number of each priority done:\n 1: %d, 2: %d, 3: %d, 4: %d\n", 
            num_hpf_entries_done[0],num_hpf_entries_done[1],num_hpf_entries_done[2],num_hpf_entries_done[3]);  
 
    int temp;
    for (temp = 0; temp < 4; ++temp) {
        metrics_array[0][temp] = 0;
        metrics_array[1][temp] = 0;
        metrics_array[2][temp] = 0;
        for (i = 0; i < num_hpf_entries_done[temp]; ++i) {
            target_entry = hpf_job_index_queues[temp][i];
            metrics_array[0][temp] += job_array[target_entry].end_time - job_array[target_entry].arrival_time;
            metrics_array[1][temp] += job_array[target_entry].end_time - job_array[target_entry].arrival_time 
                                        - job_array[target_entry].expected_run_time;
            metrics_array[2][temp] += job_array[target_entry].start_time - job_array[target_entry].arrival_time;
        }
        metrics_array[3][temp] = ((float) num_hpf_entries_done[temp]) / ((float) qi);  
        if(num_hpf_entries_done[temp] == 0) {
            metrics_array[0][temp] = -1;
            metrics_array[1][temp] = -1;
            metrics_array[2][temp] = -1;
            metrics_array[3][temp] = 0;
        } else {
            metrics_array[0][temp] = metrics_array[0][temp] / num_hpf_entries_done[temp];
            metrics_array[1][temp] = metrics_array[1][temp] / num_hpf_entries_done[temp];
            metrics_array[2][temp] = metrics_array[2][temp] / num_hpf_entries_done[temp];
        }
    }
    printf("Average turnaround time per priority: (-1 means N/A)\n 1: %6.2f, 2: %6.2f, 3: %6.2f, 4: %6.2f\n",
        metrics_array[0][0], metrics_array[0][1], metrics_array[0][2], metrics_array[0][3]);
    printf("Average waiting time per priority: (-1 means N/A)\n 1: %6.2f, 2: %6.2f, 3: %6.2f, 4: %6.2f\n",
        metrics_array[1][0], metrics_array[1][1], metrics_array[1][2], metrics_array[1][3]);
    printf("Average response time per priority: (-1 means N/A)\n 1: %6.2f, 2: %6.2f, 3: %6.2f, 4: %6.2f\n",
        metrics_array[2][0], metrics_array[2][1], metrics_array[2][2], metrics_array[2][3]);
    printf("Throughput per priority:\n 1: %6.2f, 2: %6.2f, 3: %6.2f, 4: %6.2f\n",
        metrics_array[3][0], metrics_array[3][1], metrics_array[3][2], metrics_array[3][3]);

    return 0;
}
