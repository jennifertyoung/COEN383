#include <stdio.h>
#include <stdlib.h>

#define NUM_JOBS 90
#define ALG_RUNS 5
#define QUANTA 100

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
            int gap_quanta = (int)(right_arrival_time+0.5) - (int)(left_finish_time+0.5);
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


// Helper function to compare the remaining times of two jobs
int compare_remaining_times(const void * a, const void * b)
{
    job *j1 = (job*)a;
    job *j2 = (job*)b;

    // calculate the remaining time for the two jobs
    float j1_remaining_time = j1->expected_run_time - j1->accum_run_time;
    float j2_remaining_time = j2->expected_run_time - j2->accum_run_time;

    // compare the remaining times for the two jobs
    float reminaing_time_delta = j1_remaining_time - j2_remaining_time;
    if (reminaing_time_delta > 0)
      return 1;
    if (reminaing_time_delta < 0)
      return -1;

    return reminaing_time_delta;
}

/* Function srt returns a pointer to the job that should be given CPU next.
Inputs:
  job_array - pointer to the array of jobs
  quanta  - current time slice
  started - flag set to confirm only started processes will be considered
  last_job - pointer to the last job that had the CPU
*/
job * srt(job* job_array, int quanta, int started, job* last_job)
{
  int i = 0;
  qsort(job_array, NUM_JOBS, sizeof(job), compare_remaining_times);

  for (i = 0; i < NUM_JOBS; i++)
  {
    float remaining_time = job_array[i].expected_run_time - job_array[i].accum_run_time;
    if(started)
    {
      if (quanta >= job_array[i].arrival_time && remaining_time > 0
          && job_array[i].expected_run_time != remaining_time) return &job_array[i];
    }
    else
    {
      if (quanta >= job_array[i].arrival_time && remaining_time > 0) return &job_array[i];
    }
  }
  return NULL;
}

int simulator(job* job_array, job* (*func)(job*, int, int, job*))
{
  int i = 0;
  int j = 0;
  int jobs_complete = 0;
  int total_time = 0;

  // values required for calculating run statistics
  float current_idle_time = 0;
  float current_turnaround_time = 0;
  float current_waiting_time = 0;
  float current_response_time = 0;

  // total stat values
  float total_idle_time = 0;
  float total_turnaround_time = 0;
  float total_waiting_time = 0;
  float total_response_time = 0;


  job* current_job = NULL;

  // sort based on remaining time (can reuse this code for shortest job first if desired)
  // just need top change the sorting function within qsort
  qsort(job_array, NUM_JOBS, sizeof(job), compare_remaining_times);

  printf("Expected Order of Jobs: \n");
  print_all_job_fields(job_array);
  printf("SRT Simulation beginning...\n");
  for (i = 0; i < QUANTA; i++)
  {
      // at each time slice, resort the jobs wrt their remaining times
      qsort(job_array, NUM_JOBS, sizeof(job), compare_remaining_times);
      // measure idle time for this particular quanta
      current_idle_time = 0;
      // check to see if all jobs are completed
      if (jobs_complete == NUM_JOBS)
        break;
      // if jobs still need to be run, reassign a current job
      current_job = func(job_array, i, 0, current_job);

      // if there is no current job available to run, increment total_idle_time
      if (current_job == NULL)
      {
        total_idle_time += 1.0;
        continue;
      }

      // update response time as soon as a job gets the CPU for the first time ie
      // when remaining runtime == exepected runtime
      // where remaining runtime = job's expecetd runtime - job's accumulated runtime
      if ((current_job->expected_run_time - current_job->accum_run_time) == current_job->expected_run_time)
      {
        // response time = current time slice - job's arrival time
        current_response_time = i - current_job->arrival_time;
        total_response_time += current_response_time;
      }
      // current job on CPU, so we add one time slice to its accumulated runtime.
      current_job->accum_run_time += 1;

      // if job is finished, we update statistics
      if (current_job->expected_run_time - current_job->accum_run_time <= 0)
      {
        jobs_complete++;
        // the time the job accumulated beyond what its expected runtime is the time
        // that the job was idle. ie idle time = abs(expected - accumulated)
        current_idle_time = (-1) * (current_job->expected_run_time - current_job->accum_run_time);
        // set job's remaining time equal to 0. ie accumulated runtime = expected runtime
        current_job->accum_run_time = current_job->expected_run_time;
        // update turnaround statistics
        current_turnaround_time = (i+1) - current_job->arrival_time;
        total_turnaround_time += current_turnaround_time;
        // update waiting statistics
        current_waiting_time = current_turnaround_time - current_job->expected_run_time;
        total_waiting_time += current_waiting_time;
      }

      // if CPU was idle for this time slice, update idle time
      if (current_idle_time)
        total_idle_time += current_idle_time;
    }

      // if all jobs are completed, add the remaining quanta to the total idle time
    for (j = i; j < QUANTA; j++)
      total_idle_time += 1.0;

    while ((current_job = func(job_array, i, 1, current_job)) != NULL)
    {
      current_job->accum_run_time += 1;
      // if job is now complete, record cpu idle time
      if ((current_job->expected_run_time - current_job->accum_run_time) <= 0)
      {
        jobs_complete++;
        // the time the job accumulated beyond what its expected runtime is the time
        // that the job was idle. ie idle time = abs(expected - accumulated)
        current_idle_time = (-1) * (current_job->expected_run_time - current_job->accum_run_time);
        // set job's remaining time equal to 0. ie accumulated runtime = expected runtime
        current_job->accum_run_time = current_job->expected_run_time;
        // update turnaround statistics
        current_turnaround_time = (i+1) - current_job->arrival_time;
        total_turnaround_time += current_turnaround_time;
        // update waiting statistics
        current_waiting_time = current_turnaround_time - current_job->expected_run_time;
        total_waiting_time += current_waiting_time;
      }
      // if CPU was idle for this time slice, update idle time
      if (current_idle_time)
        total_idle_time += current_idle_time;
      i++;
    }
    total_time = i;

    if (NUM_JOBS - jobs_complete == 0)
      printf("All jobs have completed after %d time slices.\n", total_time);
    else
      printf("%d jobs are incomplete after %d time slices.\n", NUM_JOBS-jobs_complete, total_time);

    printf("Average Turnaround Time: %3.1f time slices\n", total_turnaround_time / jobs_complete);
    printf("Average Waiting Time: %3.1f time slices\n", total_waiting_time / jobs_complete);
    printf("Average Response Time: %3.1f time slices\n", total_response_time / jobs_complete);
    printf("Throughput: %3.3f\n", (float)jobs_complete/(float)total_time);
    printf("CPU was idle %3.1f time slices \n", total_idle_time);

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


    //adding my shit here to run -------------------------------------------
    printf("Running Shortest Remaining Time Algorithm 5 times...");
    int x;
    for (x = 0; x < ALG_RUNS; x++)
    {
      printf("Run #%d\n", x);
      simulator(job_array, srt);
    }

    return 0;
}
