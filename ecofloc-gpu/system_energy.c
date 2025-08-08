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

#include "system_energy.h"
#include "results_map.h"


static pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;


volatile sig_atomic_t stay_running = 1; 
void handle_signal(int sig) 
{
    stay_running = 0;
}

double system_energy(int interval_ms, int timeout_s)
{
    double total_energy = 0.0;
    double interval_s = interval_ms / 1000.0; // Convert ms to seconds

    // Setup nanosleep interval
    struct timespec interval_time;
    interval_time.tv_sec = interval_ms / 1000;
    interval_time.tv_nsec = (interval_ms % 1000) * 1000000;

    unsigned long long start_time = time(NULL);

    signal(SIGINT, handle_signal);

    // 68 years :)
    if (timeout_s < 0) timeout_s = INT_MAX;

    // Convert everything to milliseconds to compute the number of iterations
    int total_iterations = (int)(timeout_s * 1000.0 / interval_ms);

    /*
     * PATCH: Iterate a fixed number of cycles computed from timeout and interval.
     * This avoids inconsistencies when each cycle may take longer than the target interval.
     */

    int iteration = 1;
    while (stay_running && iteration <= total_iterations)
    {
        pthread_mutex_lock(&fn_mutex); // Protect power readings
        float initial_power = gpu_power(); // System-wide GPU power (Watts)
        pthread_mutex_unlock(&fn_mutex);

        nanosleep(&interval_time, NULL); // Sleep for interval_time

        pthread_mutex_lock(&fn_mutex); // Protect power readings
        float final_power = gpu_power(); // System-wide GPU power (Watts)
        pthread_mutex_unlock(&fn_mutex);

        // Average power over the interval
        float avg_interval_power = (initial_power + final_power) / 2.0f;

        // Energy = power * time (seconds) -> Joules
        double interval_energy = avg_interval_power * interval_s;

        //We fixed "-333" as the system identifier in the results file. 
        int pid_for_system=-333;
        write_results(pid_for_system, time(NULL) - start_time, avg_interval_power, interval_energy, iteration, interval_ms);

        total_energy += interval_energy;
        iteration++;
    }

    return total_energy; // Total system energy in Joules
}
