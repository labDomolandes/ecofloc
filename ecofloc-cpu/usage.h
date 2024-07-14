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


#ifndef USAGE_H
#define USAGE_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>


/*
*  Description:  There are functions to retrieve the percentage of CPU usage by a process:
*                pid_cpu_usage: pid_cpu_time / total_cpu_time over a specified time interval.
*/
 
unsigned long long total_cpu_time();
unsigned long long pid_cpu_time(int pid);

/*
*  Description: This function is deprecated as the calculation is done in the main loop in pid_energy.c.
*/
double get_pid_cpu_usage(int pid, int interval_ms);



#endif //USAGE_H