 
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


/*
* POWER EQUATION W=P*C*V*V*F
* 1) P=Percentage of time that the CPU gives to the process and its threads
* 2) C=Capacitance
* 3) V=Current voltage 
* 4) F=Current frequency of involved cores (if threads)
*/


#include "usage.h"
#include <signal.h>
#include <string.h>
#include <limits.h>

#include "cpu_map.h"
#include "cpu.h"


/*
* From cpu.h
*/
extern cpu_features cpu_specs;

/*
 * Description: The function computes the CPU capacitance using the equation: 
 *              C = (0.7 * TDP) / (Frequency_TDP * Voltage_TDP^2).
 * Limitations: The only value obtained from the OS is frequency_TDP, from /proc/cpuinfo. 
 *              TDP and Voltage_TDP should also be obtained dynamically in future improvements.
 */
double capacitance();
/*
 * Description: The function computes the power consumption of the REAL cores utilized by a PID 
  * (does not consider PID cpu time yet).
 * Returns: Total power consumption in watts.
 */
double system_power(cpu_map *map);

/*
 * Variable and function for exiting the while when CNTRL+C is pressed 
*/
extern volatile sig_atomic_t stay_running;  
void handle_signal(int sig);

/*
 * Description: This function estimates the energy consumed on the CPU during a timeout period.
 * It samples CPU power at the beginning and end of each interval for the cores in use.
 * The average power is then multiplied by the CPU time measured during the period.
 * Limitations: The set of active cores may change during the measurement period.
 */

double system_energy(int interval_ms, int timeout_s);






#endif 
