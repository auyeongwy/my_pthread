/* Copyright 2018 Au Yeong Wing Yau

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

/** @file my_pthread.c
 *  Reference code that demonstrates use of the pthreads. 
 * In this implementation, a parent spawns seveal workers which waits for input from the parent thread. 
 * The parent assigns work to the workers via a work queue system. The workers exit only after they receive an exit command from the parent thread.
 * 
 * To run, my_pthread <number of threads>
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include "worker.h"


static int v_num_threads = 0; /**< Number of threads. */
static pthread_t *__restrict__ v_thread_array = NULL; /**< Threads of the applicaiton. */
static pthread_mutex_t *v_mutex_array = NULL; /**< Mutexes for each thread. */
static struct s_sync_buf *v_sync_array = NULL; /**< Sync buffer for each thread. */


/** Check command line parameters.
 *  @param p_argc Number of parameters from the commandline.
 *  @param p_argv List of commandline parameters.
 *  @return Number of threads to create. Else -1 if error.
 */
static int process_params(const int p_argc, char *const __restrict__ p_argv[]);

/**Allocate all the system memory and start the threads.
 * @return 0 if success, else -1.
 */
static int do_init();

/** Cleans up all memory allocated.
 */
static void do_cleanup(); 

/** Assign work into the worker queues.
 *  @param p_value A random value to generate work values to be processed by the workers.
 */
static void assign_work(const unsigned int p_value);

/** Cause all workers to stop work.
 */
static void stop_workers();

/** The main() function of the application.
 *  @param argc Number of parameters from the commandline.
 *  @param argv List of commandline parameters.
 *  @return 0 if application executed successfully. Else -1.
 */
int main(int argc, char *argv[])
{
	if((v_num_threads = process_params(argc, argv)) == -1) /* Checks command line arguments. */
		return -1;	
	if(do_init() == -1) { /* Memory allocation and launches the threads. */
		do_cleanup();
		return -1;
	}
	
	printf("Number of threads: %d\n", v_num_threads);
	
	assign_work(10); /* Assign work to the worker threads. */
	assign_work(20);
	assign_work(3);
	
	stop_workers(); /* Stop and cleanup. */
	do_cleanup();
	return 0;
}



static int do_init()
{
	int i;
	
	if((v_thread_array = (pthread_t*)malloc(sizeof(pthread_t) * v_num_threads)) == NULL) {
		perror(NULL);
		return -1;
	}
	
	if((v_mutex_array = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * v_num_threads)) == NULL) {
		perror(NULL);
		return -1;
	} 
	
	
	if((v_sync_array = (struct s_sync_buf*)malloc(sizeof(struct s_sync_buf) * v_num_threads)) == NULL) {
		perror(NULL);
		return -1;
	}
	
	
	for(i=0; i<v_num_threads; i++) {
		if(pthread_mutex_init(v_mutex_array+i, NULL) != 0) {
			perror(NULL);
			return -1;
		} else
			v_sync_array[i].mutex = v_mutex_array + i;
	}

	
	for(i=0; i<v_num_threads; i++) { /* Launch the worker threads. */
		v_sync_array[i].id = i+1;
		v_sync_array[i].action = NO_WORK; /* Workers have no work when they start. */
		v_sync_array[i].mutex = v_mutex_array + i;
		if(pthread_create(v_thread_array+i, NULL, init_worker, (void*)(v_sync_array+i)) != 0) {
			perror(NULL);
			return -1;
		}
	}
	
	return 0;
}



static void do_cleanup()
{
	int i;
	
	if(v_sync_array != NULL)
		free(v_sync_array);
	if(v_mutex_array != NULL) {
		for(i=0; i<v_num_threads; i++)
			pthread_mutex_destroy(v_mutex_array+i);
		free(v_mutex_array);
	}
	if(v_thread_array != NULL)
		free(v_thread_array);	
}



static void assign_work(const unsigned int p_value)
{
	int i, sync_flag;
	
	for(i=0; i<v_num_threads; i++) {
		sync_flag = 1;
		while(sync_flag) {
			pthread_mutex_lock(v_sync_array[i].mutex);
			if(v_sync_array[i].action == NO_WORK) { /* Only assign work when there is no work in the worker's queue. */
				v_sync_array[i].action = WORK_IN_QUEUE;
				v_sync_array[i].value = p_value+i;
				pthread_mutex_unlock(v_sync_array[i].mutex);
				sync_flag = 0;
			} else {
				pthread_mutex_unlock(v_sync_array[i].mutex); /* There is still work in the queue. Yield the thread and try again later. */
				//printf("Wait on worker %d\n", i+1);
				sched_yield();
				usleep(10);
			}
		}
	}	
}



static void stop_workers()
{
	int i, sync_flag;
	
	for(i=0; i<v_num_threads; i++) {
		sync_flag = 1;
		while(sync_flag) {
			pthread_mutex_lock(v_sync_array[i].mutex);
			if(v_sync_array[i].action == NO_WORK) { /* Only exit when there is no work in the worker's queue. */
				v_sync_array[i].action = EXIT;
				pthread_mutex_unlock(v_sync_array[i].mutex);
				sync_flag = 0;
			} else {
				pthread_mutex_unlock(v_sync_array[i].mutex); /* There is still work in the queue. Yield the thread and try again later. */
				sched_yield();
				usleep(10);
			}
		}
	}
	
	for(i=0; i<v_num_threads; i++) /* Wait for all threads to exit. */
		pthread_join(v_thread_array[i], NULL);	
	printf("All worker threads exited\n");
}



static int process_params(const int p_argc, char *const __restrict__ p_argv[])
{
	int num_threads;
	
	if(p_argc < 2) {
		printf("Usage: my_pthread <number of threads>\n");
		return -1;
	}
	
	if((num_threads = atoi(p_argv[1])) < 1) {
		printf("Usage: my_pthread <number of threads>\n");
		return -1;		
	}
	
	return num_threads;
}