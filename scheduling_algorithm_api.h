/*
 * scheduling_algorithm_api.h
 * API for interacting with job array from scheduling algorithm
*/

#ifndef scheduling_algorithm_api_h
#define scheduling_algorithm_api_h

typedef struct _job
{
    float arrival_time; /* random between 0.0 and 99.0 */
    float expected_run_time; /* random float between 0.1 through 10 quanta */
    int priority; /* integer 1 to 4 where 1 is highest */
    float start_time;  /*quantum when job started running for the first time. use float for easy subtraction from end time*/
    float accum_run_time; /* accum time slices */
    float end_time; /* tracks when job is complete */
    int jobnum; /*Identifier for job that corresponds with job index in sorted job_array*/
    int done; /*1 if job is complete, 0 otherwise*/
    int started; /*1 if job started, 0 otherwise*/
    int sched_allowed; /*1 if job is allowed to be scheduled, 0 otherwise*/
} job;

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
//Only valid at the start of the quantum before any new jobs have been scheduled
int get_new_job_index_range(int quantum, int *lower_new, int *upper_new);

//Whatever scheduling algorithm is running needs to decide which job is running at a particular quantum
//This function will update the job array with the start time, accum_run_time, done_flag for the requested
//job at the requested quantum.
//It will also output that job num into the quanta chart file for that run
//Any attempt to start a job that has never run before (new job) at a quantum > 99 will return an error
//Post-condition: Returns 0 for success, nonzero otherwise
int sched_job_at_quantum(int job_index, int quantum);

//Input: Job index
//Output: Remaining run time for that job. Returns 0 if remaining time is valid, otherwise
//remaining time is undefined
int get_remaining_run_time(int job_index, float * rem_time);

//Gets theoretical max
int get_theoretical_max_quantum_for_job_array();

//Returns 1 if unfinished, 0 otherwise
int unfinished_job(int quantum, int ji);

//Get quantum after which we stop scheduling new jobs
int get_quantum_stop_scheduling_value();
#endif
