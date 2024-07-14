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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // For nanosleep
#include <limits.h>
#include <signal.h>
#include "gpu.h"




extern volatile sig_atomic_t keep_running; // for the CNTRL + C

void handle_sigint(int sig);
int gpu_usage(int pid);
/*
 * Description: It calculates estimated power usage of a specific PID on GPU.
 * Retrieves total GPU power draw and process's GPU utilization percentage.
 * Estimates process's power by multiplying total power by utilization percentage.
 * Power in watts, as watts denote energy per unit time, remains consistent.
 * Even if sampling interval is not one second, watts reflect average energy rate.
 * GPU utilization is considered as the Streaming Multiprocessor (SM) percentage 
 * from nvidia-smi pmon
 * https://developer.download.nvidia.com/compute/DCGM/docs/nvidia-smi-367.38.pdf
 */


double pid_energy(int pid, int interval_ms, int timeout_s);


#endif // PID_ENERGY_H
