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
 

#include"pid_energy.h"
#include "results_map.h"

static pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;


int gpu_usage(int pid)
{
    char command[] = "nvidia-smi pmon -s um -c 1";
    char line[1024];
    int usage = 0;  
    FILE *fp;

    fp = popen(command, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to run nvidia-smi\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        int gpu_index;
        int current_pid;
        char sm_usage_str[10];  
        char type[10];          

        /*
        * # gpu         pid  type    sm    mem    enc    dec    command
        * # Idx           #   C/G     %      %      %      %    name
        *    0       2147     G      7      1      -      -    Xorg           
        */

        if (sscanf(line, "%d %d %s %s", &gpu_index, &current_pid, type, sm_usage_str) == 4)
        
        {
            if (current_pid == pid && strcmp(type, "G") == 0) // G -> GPU
            {
                if (strcmp(sm_usage_str, "-") != 0)  // IF NOT "-"
                    usage = atoi(sm_usage_str);  // Convert valid usage string to int
                else 
                    usage = 0;
                break;
            }
        }
    }
    pclose(fp);

    return usage;
}

volatile sig_atomic_t keep_running = 1; 
void handle_sigint(int sig) 
{
    keep_running = 0;
}
double pid_energy(int pid, int interval_ms, int timeout_s)
{
    double total_energy = 0.0;
    double interval_s = interval_ms / 1000.0; // Convert ms to seconds

    // Setup nanosleep interval
    struct timespec interval_time;
    interval_time.tv_sec = interval_ms / 1000;
    interval_time.tv_nsec = (interval_ms % 1000) * 1000000;

    unsigned long long start_time = time(NULL);

    signal(SIGINT, handle_sigint);

    //68 years :)
    if (timeout_s < 0) timeout_s = INT_MAX;

    while (keep_running && (time(NULL) - start_time) < timeout_s)
    {
        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 
        float initial_power = gpu_power();
        int initial_usage = gpu_usage(pid);
        float initial_process_power = (initial_usage / 100.0) * initial_power;
        pthread_mutex_unlock(&fn_mutex);

        nanosleep(&interval_time, NULL); // Sleep for interval_time

        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 
        float final_power = gpu_power();
        int final_usage = gpu_usage(pid);
        float final_process_power = (final_usage / 100.0) * final_power;
        pthread_mutex_unlock(&fn_mutex);

        //  average power over the interval
        float avg_interval_power = (initial_process_power + final_process_power) / 2.0;

        // energy = power * time (in seconds)
        double interval_energy = avg_interval_power * interval_s;

        // Write results
        write_results(pid, time(NULL) - start_time, avg_interval_power, interval_energy);

        total_energy += interval_energy;
    }

    return total_energy; // Total energy in Joules
}