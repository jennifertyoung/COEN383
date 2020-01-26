#include <stdio.h>
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

void hpfp_schedule(job * init_job_array) {
    int current_quanta = 0;
    int last_viewed_index = 0;
    int queue_complete[4] = {0,0,0,0};
    int queue_started[4] = {0,0,0,0};
    int queue_sizes[4] = {0,0,0,0};
    int prio_queues[4][NUM_JOBS]; // will hold jobnums, not jobs themselves
    
    int target_prio_queue;
    int target_job;

    int remaining_duration;    
    int quanta_chart[150]; //quanta used = current_quanta at end < 100 + 4 * 10

    int i = 0;
    int temp = 0;
    float metrics_array[4][5];    

    job job_array[NUM_JOBS];
    for (i = 0; i < NUM_JOBS; ++i){
        job_array[i] = init_job_array[i];
    }
    
    for (current_quanta = 0; current_quanta < 100; ++current_quanta) { 
        // fill prio queues with new arrivals
        while (last_viewed_index < NUM_JOBS && job_array[last_viewed_index].arrival_time <= current_quanta) {
            target_prio_queue = job_array[last_viewed_index].priority - 1; // subtract 1 as prio is 1 to 4
            prio_queues[target_prio_queue][queue_sizes[target_prio_queue]] 
                = job_array[last_viewed_index].jobnum;
            ++queue_sizes[target_prio_queue];
            ++last_viewed_index;
        }   
    
        // check who should be executed
        target_prio_queue = 0;
        while(target_prio_queue < 3 && queue_complete[target_prio_queue] >= queue_sizes[target_prio_queue]) {
            ++target_prio_queue;
        }
        if (queue_complete[target_prio_queue] < queue_sizes[target_prio_queue]) { // somthing is in the queue
            target_job = prio_queues[target_prio_queue][queue_complete[target_prio_queue]];
       
            // run that job for one quanta
            quanta_chart[current_quanta] = target_job;            
            if (job_array[target_job].start_time == 0.0) { // job is first run this quanta
                job_array[target_job].start_time = current_quanta;
                ++queue_started[target_prio_queue];
            }
            ++job_array[target_job].accum_run_time;
            if(job_array[target_job].accum_run_time >= job_array[target_job].expected_run_time) {
                job_array[target_job].end_time = current_quanta + 1; 
                ++queue_complete[target_prio_queue]; 
            }
        }
        else { // dont run anything
            quanta_chart[current_quanta] = -1;
        }
    }

    // jobs finishing past 100 quanta
    for(target_prio_queue = 0; target_prio_queue < 4; ++target_prio_queue) {
        if (queue_started[target_prio_queue] != queue_complete[target_prio_queue]) { // unfinished job present
            target_job = prio_queues[target_prio_queue][queue_complete[target_prio_queue]];
            int remaining_duration = ceil(job_array[target_job].expected_run_time 
                                        - job_array[target_job].accum_run_time); 
            job_array[target_job].accum_run_time += remaining_duration;
            job_array[target_job].end_time = current_quanta + remaining_duration;
            ++queue_complete[target_prio_queue];
            for (temp = 0; temp < remaining_duration; ++temp) {
                quanta_chart[current_quanta + temp] = target_job;
            }
            current_quanta = current_quanta + remaining_duration;
        }
    }

    /****************************/
    /*      Output Report       */
    /****************************/

    //each created process' name, arrival time, expected run time, and priority
    printf("\nprocess list:\n");
    for(temp = 0; temp < NUM_JOBS; ++temp) {
        printf("process %2d, arrive %5.2f, expected run %5.2f, prio %d\n", job_array[temp].jobnum, 
            job_array[temp].arrival_time, job_array[temp].expected_run_time, job_array[temp].priority);
    }

    //quanta chart
    printf("\nquanta chart:\n");
    for(temp = 0; temp < current_quanta; ++temp) {
        printf("%d, ", quanta_chart[temp]);
    }

    //run per queue analysis
    printf("\nper queue analysis:\n");
    
    printf("number of each priority done:\n 1: %d, 2: %d, 3: %d, 4: %d\n", 
            queue_complete[0],queue_complete[1],queue_complete[2],queue_complete[3]);  
 
    for (temp = 0; temp < 4; ++temp) {
        metrics_array[0][temp] = 0;
        metrics_array[1][temp] = 0;
        metrics_array[2][temp] = 0;
        for (i = 0; i < queue_complete[temp]; ++i) {
            target_job = prio_queues[temp][i];
            metrics_array[0][temp] += job_array[target_job].end_time - job_array[target_job].arrival_time;
            metrics_array[1][temp] += job_array[target_job].end_time - job_array[target_job].arrival_time 
                                        - job_array[target_job].expected_run_time;
            metrics_array[2][temp] += job_array[target_job].start_time - job_array[target_job].arrival_time;
        }
        metrics_array[3][temp] = ((float) queue_complete[temp]) / ((float) current_quanta);  
        if(queue_complete[temp] == 0) {
            metrics_array[0][temp] = -1;
            metrics_array[1][temp] = -1;
            metrics_array[2][temp] = -1;
            metrics_array[3][temp] = 0;
        } else {
            metrics_array[0][temp] = metrics_array[0][temp] / queue_complete[temp];
            metrics_array[1][temp] = metrics_array[1][temp] / queue_complete[temp];
            metrics_array[2][temp] = metrics_array[2][temp] / queue_complete[temp];
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

    // run overall analysis
    printf("\noverall analysis\n");
    
    temp = 0;
    metrics_array[0][4] = 0;
    metrics_array[1][4] = 0;
    metrics_array[2][4] = 0;
    for(i = 0; i < NUM_JOBS; ++i) {
        if(job_array[i].end_time > 0.0) {
            metrics_array[0][4] += job_array[i].end_time - job_array[i].arrival_time;
            metrics_array[1][4] += job_array[i].end_time - job_array[i].arrival_time 
                                    - job_array[i].expected_run_time;
            metrics_array[2][4] += job_array[i].start_time - job_array[i].arrival_time;    
            ++temp;
        }   
    }
    printf("temp %d\n", temp);
    metrics_array[0][4] = metrics_array[0][4] / temp;
    metrics_array[1][4] = metrics_array[1][4] / temp;
    metrics_array[2][4] = metrics_array[2][4] / temp;
    metrics_array[3][4] = ((float) temp) / current_quanta;
    
    printf("Average turnaround time: \n%6.2f\n", metrics_array[0][4]);
    printf("Average waiting time: \n%6.2f\n", metrics_array[1][4]);
    printf("Average response time: \n%6.2f\n", metrics_array[2][4]);
    printf("Throughput: \n%6.2f\n", metrics_array[3][4]);

    return;
}

