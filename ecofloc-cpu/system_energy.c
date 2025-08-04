 
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


#include "system_energy.h"
#include "results_map.h"


cpu_features cpu_specs;


static pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
* Base freq. source files are not standard  
*/

// double get_capacitance() 
// {
//     char buf[1024];
//     double cpu_freq_tdp; // In MHz
//     double cpu_tdp = 28.0; //FROM DATASHEET
//     double cpu_voltage_tdp = 1.5; //FROM DATASHEET
//     double cpu_capacitance;
    
//     FILE* cpuinfo_file = fopen("/proc/cpuinfo", "r");
//     if (cpuinfo_file == NULL) 
//     {
//         perror("Error opening /proc/cpuinfo");
//         return -1.0;
//     }

//     while (fgets(buf, sizeof(buf), cpuinfo_file) != NULL) 
//     {
//         if (strncmp(buf, "model name", 10) == 0) 
//         { 
//             //go to line: model name @ value GHz
//             char* frequency_str = strstr(buf, "@"); 
//             if (frequency_str != NULL) 
//             {
//                 frequency_str++;
//                 cpu_freq_tdp = strtod(frequency_str, NULL) * 1000; // Convert GHz to MHz
//                 break;
//             }
//         }
//     }
//     fclose(cpuinfo_file);
//     if (cpu_freq_tdp <= 0) 
//     {
//         fprintf(stderr, "Failed to parse CPU frequency from /proc/cpuinfo\n");
//         return -1.0;
//     }

//     cpu_capacitance = (0.7 * cpu_tdp) / (cpu_freq_tdp * cpu_voltage_tdp * cpu_voltage_tdp);
//     //printf("CAPACITANCE %f\n", cpu_capacitance);

//     return cpu_capacitance;
// }





double capacitance() 
{
    double cpu_capacitance = (0.7 * cpu_specs.cpu_tdp) / 
                             (cpu_specs.cpu_freq_tdp * 
                              cpu_specs.cpu_voltage_tdp * 
                              cpu_specs.cpu_voltage_tdp);

    return cpu_capacitance; 
}



double system_power(cpu_map *map)
{
    float total_power = 0.0;
    float cap = capacitance(); // TODO: Make capacitance global
    //printf("DEBUG: Capacitance used = %.9f\n", cap);

    /*
     * Compute allocation size for internal arrays.
     * MAX_CORE_ID may be greater than TOTAL_VCORES due to how Linux assigns core ids.
     * To avoid out-of-bound errors or segmentation faults,
     * we take the larger of the two.
     */
    int allocation_size = (map->TOTAL_VCORES > map->MAX_CORE_ID + 1) 
                          ? map->TOTAL_VCORES 
                          : (map->MAX_CORE_ID + 1);

    float *real_core_power = calloc(allocation_size, sizeof(float));
    int *real_core_count = calloc(allocation_size, sizeof(int));

     /*
     * Loop over each virtual core and compute its individual dynamic power:
     * Power = C * V^2 * f
     * Then, group this power by physical core to later compute average per core.
     */
    for (int i = 0; i < map->TOTAL_VCORES; i++)
    {
        int real_core = map->REAL_CORES[i];
        float power = cap * map->VCORE_VOLT[i] * map->VCORE_VOLT[i] * map->VCORE_FREQ[i];


         // Per-vCore debug
        float freq = map->VCORE_FREQ[i];
        float volt = map->VCORE_VOLT[i];
        //printf("DEBUG: vCore %d -> Real Core %d | Freq: %.2f MHz | Volt: %.5f V | Power: %.9f\n",
        //       i, real_core, freq, volt, power);


        real_core_power[real_core] += power;
        real_core_count[real_core]++;
    }

    /*
     * Now compute total system power:
     * For each real core used, divide total power by number of vCores sharing it.
     * This gives the average power per physical core.
     * Then sum all real core contributions to get final system-level power.
     */
    for (int i = 0; i < allocation_size; i++)
    {
        if (real_core_count[i] > 0)
        {
            float avg_power = real_core_power[i] / real_core_count[i];
            total_power += avg_power;

            //  printf("DEBUG: REAL_CORE[%d] -> Avg Power: %.9f (from %d vCores)\n",
            //        i, avg_power, real_core_count[i]);

            //  printf("DEBUG TOTAL Power: %.9f\n", total_power);

        }
    }

    free(real_core_power);
    free(real_core_count);

    return total_power;
}


volatile sig_atomic_t stay_running = 1; 
void handle_signal(int sig) 
{
    stay_running = 0;
}

double system_energy(int interval_ms, int timeout_s)
{

    cpu_map *map = malloc(sizeof(cpu_map));

    init_maps(map);
    map_cores(map);

    //68 years :)
    if (timeout_s < 0) timeout_s = INT_MAX;
    
    double total_energy = 0.0;
    //double total_power = 0.0;
    double avg_cpu_power = 0.0;
    //int iterations = 0;
 

    //All to milliseconds to get the iterations to perform
    int total_iterations = (int)(timeout_s * 1000.0 / interval_ms);


    // Convert the interval in milliseconds to a timespec struct for nanosleep
    struct timespec interval_time;
    interval_time.tv_sec = interval_ms / 1000;
    interval_time.tv_nsec = (interval_ms % 1000) * 1000000;

    signal(SIGINT, handle_signal);

    unsigned long long start_time = time(NULL);


    /*
    * PATCH: Instead of stopping the loop based on timeout expiration, EcoFloc now iterates 
    * based on the computed number of iterations. 
    * This approach prevents inconsistencies when iteration duration varies. 
    * Example: If each iteration takes longer than the specified interval, 
    * relying solely on elapsed time could cause an early exit before completing the intended cycles.
    */


    //while (keep_running && (time(NULL) - start_time) <= timeout_s)
    int iteration=1;
    while (stay_running && iteration <= total_iterations)
    {
        //Power variables and time calculations 
        map_frequencies(map);
        map_voltages(map);
         
        
        double power_start = system_power(map);
        
        // Wait to calculate the cpu time
        nanosleep(&interval_time, NULL);

        map_frequencies(map);
        map_voltages(map);

        double power_end = system_power(map);


        //System power and energy calculation 

        double avg_power_interval = (power_start + power_end) / 2.0;

        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 

        unsigned long long end_total_time = total_cpu_time();

        pthread_mutex_unlock(&fn_mutex);

        
        // Calculate the energy consumed during the interval in Joules (watts x sec)
        double interval_sec = interval_ms / 1000.0; // -> to secs

        pthread_mutex_lock(&fn_mutex); // Protect time values retrieval 

        double interval_energy = avg_power_interval * interval_sec;

        pthread_mutex_unlock(&fn_mutex);

        //We fixed "-333" as the system identifier in the results file. 
        int pid_for_system=-333;

        write_results(pid_for_system, time(NULL) - start_time, avg_power_interval, interval_energy, iteration, interval_ms);
        
        total_energy += interval_energy; //TODO-> cummulation in map

        iteration++;
    }
    
   

    return total_energy;
}


