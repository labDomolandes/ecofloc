/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef COMM_ENERGY_H
#define COMM_ENERGY_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include "pid_energy.h"
#include "cpu_map.h"


#define MAX_PIDS 1024
#define SLEEP_PID_DISCOVERING 1

/*
 * Description: These functions calculate the energy consumption of a command by measuring all its processes with
 *              the functions in pid_energy.c.
 *              To account for new processes it may launch, the pids that have already been treated are tracked,
 *              and new ones arriving are only treated if they arrive during the specified timeout period.
*/

extern int new_pids[MAX_PIDS];
extern int treated_pids[MAX_PIDS];
extern int new_pid_count;
extern int treated_pid_count;
extern double total_energy;

extern int interval_ms_global;
extern int timeout_s_global;
extern volatile int active_thread_count;

extern pthread_mutex_t mutex;
extern pthread_mutex_t energy_mutex;
extern pthread_mutex_t treated_mutex;
extern pthread_cond_t cond_var;

/*
 * Description: If 1, ecofloc continues to search for pids even when the application has closed.
 *              If 0, ecofloc stops when the pid is no longer found.
*/
extern int dynamic_mode;


int is_pid_folder(const char *name);
int get_cmd_from_pid(char *pid, char *buffer, size_t buffer_size);
int is_treated(int pid);
void *pid_energy_thread(void *arg);
void launch_energy_threads();

/*
 * Description: This is the main thread that loops until the timeout is reached, or if no more processes 
 * are found when dynamic_mode is inactive.
 */

void *discover_pids(void *arg);


void comm_energy(const char *cmd_name, int interval_ms, int timeout_s);

#endif // COMM_ENERGY_H
