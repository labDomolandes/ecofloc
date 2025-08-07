 
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
#include <time.h>
#include "results_map.h"
#include <signal.h>
#include <limits.h>

#include "nic.h"



/*
 * Variable and function for exiting the while when CNTRL+C is pressed 
*/
extern volatile sig_atomic_t satay_running;  
void handle_signal(int sig);

/*
* From nic.h
*/
extern nic_features n_features;

/*
 * We use nethogs to obtain global network statistics across all processes.
*/

double system_energy(int interval_ms, int timeout_s);



#endif 
