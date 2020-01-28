// SRT.c - Shortest Remainging Time Scheduling Algorithm
#include "project2.c"

job * srt(job* job_array, int quanta, int started, job* last_job);
int compare_remaining_times(const void * a, const void * b);
int simulator(job* job_array, job* (*func)(job*, int, int, job*));

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

int simulator(job* job_array, job* (*func)(job*, int, int, job*))
{
  int i,j = 0;
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


  job* current_job = NULL:

  // sort based on remaining time (can reuse this code for shortest job first if desired)
  // just need top change the sorting function within qsort
  qsort(job_array, NUM_JOBS, sizeof(job), compare_remaining_times);

  printf("Expected Order of Jobs: \n");
  print_all_job_fields(job_array);
  printf("SRT Simulation beginning...\n");
  for (int i = 0; i < QUANTA; i++)
  {
      // at each time slice, resort the jobs wrt their remaining times
      qsort(job_array, NUM_JOBS, sizeof(job), compare_remaining_times);
      // measure idle time for this particular quanta
      current_idle_time = 0;
      // check to see if all jobs are completed
      if (jobs_completed == NUM_JOBS)
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

      // if all jobs are completed, add the remaining quanta to the total idle time
      for (j = i; j < QUANTA, j++)
        total_idle_time += 1.0;

      while ((current_job = func(job_array, i, 1, current_job)) != NULL)
      {
        current_job->accum_run_time += 1;
        // if job is now complete, record cpu idle time
        if ((current_job->expecetd_time - current_job->accumulated) <= 0)
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
