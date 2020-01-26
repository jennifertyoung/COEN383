/*
 * scheduling_algorithm_api.h
 * API for interacting with job array from scheduling algorithm
*/

#ifndef scheduling_algorithm_api_h
#define scheduling_algorithm_api_h

//Input: quantum number
//Output: job index into job array for the lowest unfinished job inclusive at that quantum
//job index into job array for the highest unfinished job inclusive at that quantum
//Returned value 0 means range returned is valid
//Returned value is positive for undefined range
//Returned value is negative for error
//Caller needs to use is_job_done while going through this range to determine which jobs to run next
int get_unfinished_job_index_range(int quantum, int *lower, int *upper);

//Input: specified quantum number
//Output: job index for new jobs that have arrived and are available for scheduling in this quantum
//This differs from get_unfinished_job_index_range because it only returns newly arrived jobs
//Returned value 0 means range returned is valid, otherwise range returned is undefined
int get_new_job_index_range(int quantum, int *lower_new, int *upper_new);

//Input: specified quantum number
//Output: job index for jobs that previously arrived and are not done. the unfinished jobs, not including
//newly arrived jobs
//Returned value 0 means range returned is valid, otherwise range returned is undefined
int get_prior_job_index_range(int quantum, int *lower_prior, int *upper_prior);

//Returns 1 if the job specified by job_index is done, 0 otherwise
int is_job_done(int job_index);

//Returns 1 if the job specified by job_index is started, 0 otherwise
int is_job_started(int job_index);

//Whatever scheduling algorithm is running needs to decide which job is running at a particular quantum
//This function will update the job array with the start time, accum_run_time, done_flag for the requested
//job at the requested quantum.
//It will also output that job num into the quanta chart file for that run
//Any attempt to start a job that has never run before (new job) at a quantum > 99 will return an error
//Post-condition: Returns 0 for success, nonzero otherwise
int sched_job_at_quantum(int job_index, int quantum);

//Input: Job index
//Output: Remaining run time for that job
int get_remaining_run_time(int job_index, float * rem_time);

//Utility function that will prevent any new jobs from being scheduled after it is called
//int stop_scheduling_more_jobs(void);

//Input: Quantum
//Output: Every job that has arrived prior to that quantum is done
//Returns 1 if all prev jobs done at that quantum, 0 otherwise
int are_all_prev_jobs_done(int quantum);

#endif
