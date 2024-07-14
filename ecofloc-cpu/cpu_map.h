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


#ifndef CPU_MAP_H
#define CPU_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>



/* 
 * These global arrays are all indexed by virtual core numbers, 
 * with each index corresponding to a specific vcore:
 * Error value = -1
*/


typedef struct 
{
//Avoid "extern" because of the threads in by_name measuring. Its thead needs its own struct instance...
    int *REAL_CORES;
    float *VCORE_FREQ;   // Example: VCORE_FREQ[5] = 2.8 -> frequency of vcore 5 is 2.8
    float *VCORE_VOLT;
    int *PID_VCORES;
    int TOTAL_VCORES;
} cpu_map;


/*
 * Description: The func initializes the variables with the size of the number of cores.
 */
void init_maps(cpu_map* map);

/*
 * Description: Maps virtual cores (vcores) to their respective physical core IDs 
 *              using the "processor" and "core id" fields from /proc/cpuinfo.
 *              It assumes that each "processor" entry is followed by its corresponding "core id" 
 *              entry as typically presented in /proc/cpuinfo.
 */
void map_cores(cpu_map* map);

/*
 * Description: Maps each virtual core to its current frequency in VCORE_FREQ using the
 *              scaling_cur_freq (/sys/devices/system/cpu/cpu%d/cpufreq/) in MHz.
 */
void map_frequencies(cpu_map* map);

/*
 * Description: Maps each virtual core to its voltage from the MSR 0x198's bitfield 47:32.
*/
void map_voltages(cpu_map* map);

/*
 * Description: Puts 1 if the core is used by the PID.
*/
int map_pid(cpu_map* map, int pid);

#endif // CPU_MAP