 
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



#include "pid_energy.h"
#include "results_map.h"




void strip_non_digit(char *src, char *dst) 
{
    while (*src) 
    {
        if (isdigit(*src) || *src == '.')
            *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) 
{
    keep_running = 0;
}

double pid_energy(int pid, int interval_ms, int timeout_s)
{
    time_t start_time = time(NULL);
    char command[512];
    char output[1024];
    char clean_output[1024];

    double interval_s = interval_ms / 1000.0;
    double total_energy = 0.0;

    while (keep_running && (time(NULL) - start_time) < timeout_s)
    {
        // Construct the command to run perf
        sprintf(command, "perf stat -e mem-stores,mem-loads -p %d --timeout=%d 2>&1", pid, interval_ms);

        // Trigger perf
        FILE *fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to run command");
            exit(1);
        }

        // Variables to hold parsed values
        double mem_stores = 0, mem_loads = 0;
        double cpu_core_mem_stores = 0.0;
        double cpu_core_mem_loads = 0.0;
        double avg_interval_power = 0.0;
        double interval_energy = 0.0;

        // Two possible output formats in perf: with and without the "cpu_core" prefix 
        int case_type = 0;

        signal(SIGINT, handle_sigint);

        // If there is a new line in the file
        while (fgets(output, sizeof(output) - 1, fp) != NULL)
        {
            // Truncates the string if "(" is present
            char *percent_ptr = strchr(output, '(');
            if (percent_ptr != NULL)
                *percent_ptr = '\0';

            // Remove trailing whitespace
            for (int j = strlen(output) - 1; j >= 0; j--)
            {
                if (isspace(output[j]))
                    output[j] = '\0';
                else
                    break;
            }

            if (strstr(output, "cpu_core/mem-stores/") != NULL || strstr(output, "cpu_core/mem-loads/") != NULL)
            {
                case_type = 2;
                if (strstr(output, "cpu_core/mem-stores/") != NULL)
                {
                    strip_non_digit(output, clean_output);
                    sscanf(clean_output, "%lf", &cpu_core_mem_stores);
                }
                else if (strstr(output, "cpu_core/mem-loads/") != NULL)
                {
                    strip_non_digit(output, clean_output);
                    sscanf(clean_output, "%lf", &cpu_core_mem_loads);
                }
            }
            else if (strstr(output, "mem-stores") != NULL || strstr(output, "mem-loads") != NULL)
            {
                if (case_type != 2)
                {
                    case_type = 1;
                    if (strstr(output, "mem-stores") != NULL)
                    {
                        strip_non_digit(output, clean_output);
                        sscanf(clean_output, "%lf", &mem_stores);
                    }
                    else if (strstr(output, "mem-loads") != NULL)
                    {
                        strip_non_digit(output, clean_output);
                        sscanf(clean_output, "%lf", &mem_loads);
                    }
                }
            }
        }

        pclose(fp);

        // Calculate RAM active energy consumption
        double ram_act = 0.0;
        if (case_type == 1)
            ram_act = (mem_loads * 6.6) + (mem_stores * 8.7);
        else if (case_type == 2)
            ram_act = (cpu_core_mem_stores * 8.7) + (cpu_core_mem_loads * 6.6);

        // Total RAM power
        if (ram_act != 0)
        {
            interval_energy = ram_act / 1000000000; // Convert from nanoJoules to Joules
            avg_interval_power = interval_energy / interval_s; // Average interval power
        }

        // Write results
        write_results(pid, time(NULL) - start_time, avg_interval_power, interval_energy);

        total_energy += interval_energy;

        // struct timespec interval = {0, interval_ms * 1000000};
        // nanosleep(&interval, NULL);
    }

    return total_energy; // Total energy in Joules
}
