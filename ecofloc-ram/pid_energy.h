/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

#ifndef PID_ENERGY_H
#define PID_ENERGY_H


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // For nanosleep
#include <limits.h>
#include <ctype.h>

extern volatile sig_atomic_t keep_running; // for the CNTRL + C


/*
* The output of PERF give an space for thousands separator. This function deletes this space
*/
void strip_non_digit(char *src, char *dst);
void handle_sigint(int sig);

/*
 * Description: It calculates the estimated power usage of a specific PID on RAM.
 * It retrieves the amount of read and write operations of the PID in RAM and 
 * calculates the power consumption considering the energy value for each operation.
 */
double pid_energy(int pid, int interval_ms, int timeout_s);



#endif // PID_ENERGY_H
