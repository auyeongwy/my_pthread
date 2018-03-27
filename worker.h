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



/** @file worker.h
 *  Header file for a worker thread.
 */

#ifndef _WORKER_H_
#define _WORKER_H_

#include <pthread.h>


/** Worker commands. */
enum WORKER_COMMAND
{
	NO_WORK, /**< No work in the queue. */
	WORK_IN_QUEUE, /**< Work in the queue. */
	REST, /**< Worker to rest a while. */
	EXIT /**< Worker to stop working. */
};


/** A buffer containing synchonization memory values. */
struct s_sync_buf
{
	int id; /**< Id of the worker thread. */
	enum WORKER_COMMAND action; /**< Current command or state of the worker thread. */
	unsigned int value; /**< A computation value for the worker to do some work. */
	pthread_mutex_t *mutex; /**< Mutex for synchronization. */
};


/** A pthread function to be called to launch a worker thread.
 *  @param Arguments for the worker thread. Should point to an assigned struct s_sync_buf.
 */
void *init_worker(void *p_arguments);



#endif