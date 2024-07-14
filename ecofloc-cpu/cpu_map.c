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




#include "cpu_map.h"



void init_maps(cpu_map *map)
{
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) 
    {
        perror("Failed to open /proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    // Count vcores
    map->TOTAL_VCORES = 0;
    char line[256];
    while (fgets(line, sizeof(line), cpuinfo)) 
    {
        if (strncmp(line, "processor", 9) == 0) 
        {
            map->TOTAL_VCORES++;
        }
    }
    fclose(cpuinfo);

    // Allocate memory for each resource
    map->REAL_CORES = malloc(map->TOTAL_VCORES * sizeof(int));
    map->VCORE_FREQ = malloc(map->TOTAL_VCORES * sizeof(float));
    map->VCORE_VOLT = malloc(map->TOTAL_VCORES * sizeof(float));
    map->PID_VCORES = malloc(map->TOTAL_VCORES * sizeof(int));

    if (!map->REAL_CORES || !map->VCORE_FREQ || !map->VCORE_VOLT || !map->PID_VCORES) 
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
}


void map_cores(cpu_map* map) 
{
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) 
    {
        perror("Failed to open /proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    char line[256];
   
    // Reset file pointer to read again
    rewind(cpuinfo);

    int vcore_id = -1;
    int core_id = -1;

    // Match vcores <-> cores. processor and core id fields in /proc/cpuinfo 
    //...assuming that default order is present: processor before that core id 

    while (fgets(line, sizeof(line), cpuinfo)) 
    {
        if (sscanf(line, "processor : %d", &vcore_id) == 1) 
            map->REAL_CORES[vcore_id] = -1;  // Initialize with -1
        else if (sscanf(line, "core id : %d", &core_id) == 1 && vcore_id != -1) 
            map->REAL_CORES[vcore_id] = core_id;
        
    }

    fclose(cpuinfo);

    // printf("VCORES to CORES:\n");
    // for (int i = 0; i < map->TOTAL_VCORES; i++) 
    // {
    //     printf("Virtual Core %d -> Physical Core %d\n", i, map->REAL_CORES[i]);
    // }
}


void map_frequencies(cpu_map* map)
{
    char freq_path[256];
    FILE *freq_file;
    float frequency_kHz;

    for (int i = 0; i < map->TOTAL_VCORES; i++)
    {
        sprintf(freq_path, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", i);
        freq_file = fopen(freq_path, "r");
        if (!freq_file)
        {
            perror("Failed to open frequency file");
            map->VCORE_FREQ[i] = -1.0;  
            continue;
        }

        if (fscanf(freq_file, "%f", &frequency_kHz) == 1)
            map->VCORE_FREQ[i] = frequency_kHz / 1000.0;  // To MHz ... TODO -> TELL WHY
        else
            map->VCORE_FREQ[i] = -1.0;  

        fclose(freq_file);
    }

    // for (int i = 0; i <map->TOTAL_VCORES; i++) 
    //     printf("VCORE %d -> FREQ %f\n", i,map->VCORE_FREQ[i]);
}





void map_voltages(cpu_map* map)
{
    char command[256];
    FILE *fp;
    char voltage_buffer[256];  // Buffer to read voltage data into

    for (int i = 0; i < map->TOTAL_VCORES; i++)
    {
        
        sprintf(command, "sudo rdmsr -p%d 0x198 -u --bitfield 47:32", i);

        fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to run rdmsr command");
            map->VCORE_VOLT[i] = -1.0; 
            continue;
        }

        if (fgets(voltage_buffer, sizeof(voltage_buffer), fp) != NULL)
            map->VCORE_VOLT[i] = atof(voltage_buffer) / 8192.0;  // Adjust voltage according to rdmsr scaling
        else
            map->VCORE_VOLT[i] = -1.0;  // Use -1.0 to indicate an error in parsing the voltage


        pclose(fp);
    }

    //  for (int i = 0; i < map->TOTAL_VCORES; i++) 
    //     printf("VCORE %d -> VOLT %f\n", i, map->VCORE_VOLT[i]);
}

int map_pid(cpu_map* map, int pid)
{

    char stat_path[256];
    DIR *task_dir;
    struct dirent *task_dirent;

    // Reset PID_VCORES array to 0 at the beginning
    memset(map->PID_VCORES, 0, sizeof(map->PID_VCORES[0]) * map->TOTAL_VCORES);

    sprintf(stat_path, "/proc/%d/task", pid);
    task_dir = opendir(stat_path);

    if (task_dir == NULL) 
    {
        printf("PID %d CLOSED OR INEXISTENT\n",pid);
        return 1;
    }

    while ((task_dirent = readdir(task_dir)) != NULL) 
    {
        if (task_dirent->d_type == DT_DIR && atoi(task_dirent->d_name) > 0) 
        {
            int cpu;
            sprintf(stat_path, "/proc/%d/task/%s/stat", pid, task_dirent->d_name);
            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file) 
            {
                // Read the CPU number from the stat file
                fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*llu %*llu %*d %*d %*d %*d %*d %*d %*llu %*u %*d %*d %*d %*d %*d %*d %*d %d", &cpu);
                fclose(stat_file);
                if (cpu >= 0 && cpu < map->TOTAL_VCORES) {
                    map->PID_VCORES[cpu] = 1;  // Set the vCPU as used
                }
            }
        }
    }
    closedir(task_dir);

    return 0;


    // printf("CPUs used by PID %d:\n", pid);
    // for (int i = 0; i < map->TOTAL_VCORES; i++) {
    //     if (map->PID_VCORES[i] == 1) {
    //         printf("PID_CPU-%d\n", i);
    //     }
    // }

}