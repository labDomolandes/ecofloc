 
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

#ifndef SYSTEM_ENERGY_H
#define SYSTEM_ENERGY_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // For nanosleep
#include <limits.h>
#include <ctype.h>


/*
 * Variable and function for exiting the while when CNTRL+C is pressed 
*/
extern volatile sig_atomic_t stay_running;  
void remove_non_digit(char *src, char *dst);
void handle_signal(int sig);

/*
 * Description: It calculates the estimated energy usage of the entire system's RAM.
 * It retrieves the global amount of memory read and write operations performed by all processes,
 * and calculates the power consumption considering the energy cost per operation.
 */
double system_energy(int interval_ms, int timeout_s);






#endif 

