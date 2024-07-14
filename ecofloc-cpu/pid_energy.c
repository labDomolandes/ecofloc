 
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


/*
* EQUATION W=P*C*V*V*F
* 1) P=Percentage of time that the CPU gives to the process and its threads
* 2) C=Capacitance
* 3) V=Current voltage 
* 4) F=Current frequency of involved cores (if threads)
*/


#include "pid_energy.h"
#include "results_map.h"

static pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;

double get_capacitance() 
{
    char buf[1024];
    double cpu_freq_tdp; // In MHz
    double cpu_tdp = 28.0; //FROM DATASHEET
    double cpu_voltage_tdp = 1.5; //FROM DATASHEET
    double cpu_capacitance;
    
    FILE* cpuinfo_file = fopen("/proc/cpuinfo", "r");
    if (cpuinfo_file == NULL) 
    {
        perror("Error opening /proc/cpuinfo");
        return -1.0;
    }

    while (fgets(buf, sizeof(buf), cpuinfo_file) != NULL) 
    {
        if (strncmp(buf, "model name", 10) == 0) 
        { 
            //go to line: model name @ value GHz
            char* frequency_str = strstr(buf, "@"); 
            if (frequency_str != NULL) 
            {
                frequency_str++;
                cpu_freq_tdp = strtod(frequency_str, NULL) * 1000; // Convert GHz to MHz
                break;
            }
        }
    }
    fclose(cpuinfo_file);
    if (cpu_freq_tdp <= 0) 
    {
        fprintf(stderr, "Failed to parse CPU frequency from /proc/cpuinfo\n");
        return -1.0;
    }

    cpu_capacitance = (0.7 * cpu_tdp) / (cpu_freq_tdp * cpu_voltage_tdp * cpu_voltage_tdp);
    //printf("CAPACITANCE %f\n", cpu_capacitance);

    return cpu_capacitance;
}



double current_power(cpu_map *map)
{


    float total_power = 0.0;
    float capacitance= get_capacitance(); //TODO:  Make capacitance global to calculate it only once
     
    // map to cumulate the power of each real core
    float *real_core_power = calloc(map->TOTAL_VCORES, sizeof(float));
    // number of vCores sharing each real core. For all cases I know, that is 2 (hyperthreading),
    // but...one never knows :)
    int *real_core_count = calloc(map->TOTAL_VCORES, sizeof(int));

    for (int i = 0; i < map->TOTAL_VCORES; i++)
    {
        if (map->PID_VCORES[i] == 1)
        {
            int real_core = map->REAL_CORES[i];
            float power = capacitance * map->VCORE_VOLT[i] * map->VCORE_VOLT[i] * map->VCORE_FREQ[i];
            real_core_power[real_core] += power;
            real_core_count[real_core]++;
        }
    }

    for (int i = 0; i < map->TOTAL_VCORES; i++)
    {
        if (real_core_count[i] > 0)
        {
            // Average the power for vCores sharing the same real core
            total_power += real_core_power[i] / real_core_count[i];
        }
    }

    //printf("Total CPU Power: %.2f\n", total_power);

    free(real_core_power);
    free(real_core_count);

    return total_power;

}

volatile sig_atomic_t keep_running = 1; 
void handle_sigint(int sig) 
{
    keep_running = 0;
}

double pid_energy(int pid, int interval_ms, int timeout_s)
{


    cpu_map *map = malloc(sizeof(cpu_map));

    int map_pid_error = 0;

    init_maps(map);
    map_cores(map);

    //68 years :)
    if (timeout_s < 0) timeout_s = INT_MAX;

    unsigned long long start_pid_time = pid_cpu_time(pid);
    unsigned long long start_total_time = total_cpu_time();
    
    double total_energy = 0.0;
    //double total_power = 0.0;
    double avg_cpu_power = 0.0;
    //int iterations = 0;

    // Convert the interval in milliseconds to a timespec struct for nanosleep
    struct timespec interval_time;
    interval_time.tv_sec = interval_ms / 1000;
    interval_time.tv_nsec = (interval_ms % 1000) * 1000000;

    signal(SIGINT, handle_sigint);

    unsigned long long start_time = time(NULL);


    while (keep_running && (time(NULL) - start_time) < timeout_s)
    {
        //Power variables and time calculations 
        map_frequencies(map);
        map_voltages(map);
         
        if(map_pid(map,pid)){break;} //IF PID does not exist
        
        double power_start = current_power(map);

        // Wait to calculate the pid cpu time
        nanosleep(&interval_time, NULL);

        map_frequencies(map);
        map_voltages(map);
        if(map_pid(map,pid)){break;}//IF PID does not exist
        double power_end = current_power(map);


        //PID power and energy calculation 

        double avg_power_interval = (power_start + power_end) / 2.0;

        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 

        unsigned long long end_pid_time = pid_cpu_time(pid);
        unsigned long long end_total_time = total_cpu_time();

        pthread_mutex_unlock(&fn_mutex);


        double pid_time_diff = end_pid_time - start_pid_time;
        double total_time_diff = end_total_time - start_total_time;
        
        // Calculate CPU usage for the interval
        if (pid_time_diff > total_time_diff) 
        {
            fprintf(stderr, "Anomaly detected: PID time difference > total time difference.\n");
            continue;
        }
        double cpu_usage = (pid_time_diff / total_time_diff);
        
        // Calculate the energy consumed during the interval in Joules (watts x sec)
        double interval_sec = interval_ms / 1000.0; // -> to secs

        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 

        double interval_energy = avg_power_interval * cpu_usage * interval_sec;
        double avg_interval_power = avg_power_interval * cpu_usage;

        pthread_mutex_unlock(&fn_mutex);

        
        write_results(pid, time(NULL) - start_time, avg_interval_power,interval_energy);
        //print_results();

        total_energy += interval_energy; //TODO-> cummulation in map

        start_pid_time = end_pid_time;
        start_total_time = end_total_time;


    }
    
   

    return total_energy;
}
























































