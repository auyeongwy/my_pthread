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


/** @file worker.c
 *  Implementation of a worker thread.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include "worker.h"


/** Action taken by this thread.
 *  @param p_id The thread id.
 *  @param p_final_value A value this thread is tracking.
 *  @param p_value A random input value for the thread to perform calculation.
 *  @return The updated value this thread is monitoring.
 */
static unsigned int do_action(const int p_id, unsigned int p_final_value, unsigned int p_value);



void *init_worker(void *p_arguments)
{
	struct s_sync_buf *__restrict__ sync_buf = (struct s_sync_buf*)p_arguments;
	const int thread_id = sync_buf->id;
	enum WORKER_COMMAND action=NO_WORK;
	unsigned int value, final_value = 0;
	
	
	while(action != EXIT) {
		pthread_mutex_lock(sync_buf->mutex);
		action = sync_buf->action;
		if(action == WORK_IN_QUEUE) {
			sync_buf->action = NO_WORK; /* There is work in the queue. Copy it out and clear the queue before we start working on the assigned work. */
			value = sync_buf->value;
		} else if(action == NO_WORK) /* There is no work. Rest a bit. */
			action = REST;
		pthread_mutex_unlock(sync_buf->mutex);
		
		if(action == WORK_IN_QUEUE)
			final_value = do_action(thread_id, final_value, value);
		else if(action == REST) {
			sched_yield();
			usleep(10);
		}
	}
	
	printf("Thread %d exiting\n", thread_id);
	return NULL;
}


static unsigned int do_action(const int p_id, unsigned int p_final_value, unsigned int p_value)
{
	while(p_value > 0) 
		p_final_value += --p_value;
	
	printf("Thread %d: Updated Value: %u\n", p_id, p_final_value);
	return p_final_value;
}