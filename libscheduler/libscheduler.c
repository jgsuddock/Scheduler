/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
	int id;
	int arrival_time;
	int running_time;
	int remaining_time;
	int priority;
} job_t;

typedef struct _scheduler_t
{
	int numofcores;
	int total_wait_time;
	int total_response_time;
	int total_turnaround_time;
	int total_finished_jobs;
	job_t** core_array;
	scheme_t scheme;
	priqueue_t job_queue;
} scheduler_t;

scheduler_t* scheduler;

/*
 * Comparator return value meaning:
 *     <0 a goes before b
 *     =0 a is equivalent to b
 *     >0 a goes after b
 */

// Place new element at end of queue
int compare_fcfs(const void * a, const void * b) 
{
	return 1;
}

// Sort based on running time, then by arrival time
int compare_sjf(const void * a, const void * b) 
{
	job_t const *lhs = (job_t*) a;
	job_t const *rhs = (job_t*) b;

	int temp = lhs->running_time - rhs->running_time;

	if(temp == 0) {
		return lhs->arrival_time - rhs->arrival_time;
	}
	return temp;
}

// Sort based on remaining time, then by arrival time
int compare_psjf(const void * a, const void * b) 
{
	job_t const *lhs = (job_t*) a;
	job_t const *rhs = (job_t*) b;

	int temp = lhs->remaining_time - rhs->remaining_time;

	if(temp == 0) {
		return lhs->arrival_time - rhs->arrival_time;
	}
	return temp;
}

// Sort based on priority, then by arrival time
int compare_pri(const void * a, const void * b) 
{
	job_t const *lhs = (job_t*) a;
	job_t const *rhs = (job_t*) b;

	int temp = lhs->priority - rhs->priority;

	if(temp == 0) {
		return lhs->arrival_time - rhs->arrival_time;
	}
	return temp;
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{

	scheduler = (scheduler_t *)malloc(sizeof(scheduler_t));
	scheduler->numofcores = cores;
	scheduler->core_array = (job_t **)malloc(cores * sizeof(job_t *));

	scheduler->total_wait_time = 0;
	scheduler->total_response_time = 0;
	scheduler->total_turnaround_time = 0;
	scheduler->total_finished_jobs = 0;

	int i;
	for(i = 0; i < cores; i++) {
		scheduler->core_array[i] = NULL;
	}

	scheduler->scheme = scheme;

	switch(scheme) {
		case FCFS:
			priqueue_init(&scheduler->job_queue, compare_fcfs);
			break;

		case SJF:
			priqueue_init(&scheduler->job_queue, compare_sjf);
			break;

		case PSJF:
			priqueue_init(&scheduler->job_queue, compare_psjf);
			break;

		case PRI:
			priqueue_init(&scheduler->job_queue, compare_pri);
			break;

		case PPRI:
			priqueue_init(&scheduler->job_queue, compare_pri);
			break;

		case RR:
			priqueue_init(&scheduler->job_queue, compare_fcfs);
			break;

		default:
			priqueue_init(&scheduler->job_queue, compare_fcfs);
			break;
	}
}

/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	job_t* job = malloc(sizeof(job_t));
	job->id = job_number;
	job->arrival_time = time;
	job->running_time = running_time;
	job->remaining_time = running_time;
	job->priority = priority;

	int maxPriority = -1;
	int maxPIndex = -1;
	int maxRemainingTime = -1;
	int maxRTIndex = -1;
	int i;
	for(i = 0; i < scheduler->numofcores; i++) {
		if(scheduler->core_array[i] == NULL) {
			scheduler->core_array[i] = job;
			return i;
		}
		if (scheduler->core_array[i]->priority > maxPriority) {
			maxPriority = scheduler->core_array[i]->priority;
			maxPIndex = i;
		}
		if (scheduler->core_array[i]->remaining_time > maxRemainingTime) {
			maxRemainingTime = scheduler->core_array[i]->remaining_time;
			maxRTIndex = i;
		}
	}
	
	// If Preemptive PRI and smaller priority than max priority currently
	// running in the cores;
	if(scheduler->scheme == PPRI && job->priority < maxPriority) {
		priqueue_offer(&scheduler->job_queue, scheduler->core_array[maxPIndex]);
		scheduler->core_array[maxPIndex] = job;
		return maxPIndex;
	}
	// If Preemptive SJF and smaller remaining time than max remaining
	// time currently running in the cores;
	if(scheduler->scheme == PSJF && job->remaining_time < maxRemainingTime) {
		priqueue_offer(&scheduler->job_queue, scheduler->core_array[maxRTIndex]);
		scheduler->core_array[maxRTIndex] = job;
		return maxRTIndex;
	}

	priqueue_offer(&scheduler->job_queue, job);

	return -1;
}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	scheduler->total_finished_jobs++;
	job_t* oldjob = scheduler->core_array[core_id];
	scheduler->total_wait_time += (time - oldjob->arrival_time - oldjob->running_time);
	scheduler->total_turnaround_time += (time - oldjob->arrival_time);

	free(oldjob);

	job_t* job = priqueue_poll(&scheduler->job_queue);

	if(job == NULL) {
		scheduler->core_array[core_id] = NULL;
		return -1;
	}
	
	if(job->remaining_time == job->running_time) {
		scheduler->total_response_time += (time - job->arrival_time);
	}
	scheduler->core_array[core_id] = job;
	return job->id;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	job_t* oldjob = scheduler->core_array[core_id];

	priqueue_offer(&scheduler->job_queue, oldjob);

	job_t* newjob = priqueue_poll(&scheduler->job_queue);
	
	scheduler->core_array[core_id] = newjob;
	if(newjob == NULL) {
		return -1;
	}
	return newjob->id;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	if(scheduler->total_finished_jobs == 0) {
		return 0.0;
	}
	return (float) scheduler->total_wait_time / (float) scheduler->total_finished_jobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	if(scheduler->total_finished_jobs == 0) {
		return 0.0;
	}
	return (float) scheduler->total_turnaround_time / (float) scheduler->total_finished_jobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	if(scheduler->total_finished_jobs == 0) {
		return 0.0;
	}
	return (float) scheduler->total_response_time / (float) scheduler->total_finished_jobs;
}


/**
  Free any memory associated with your scheduler.
 
  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
