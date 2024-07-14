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

#include "usage.h"

unsigned long long total_cpu_time() 
{
    
    FILE *fp;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    fp = fopen("/proc/stat", "r");
    if(fp == NULL) 
    {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);

    return user + nice + system + idle + iowait + irq + softirq + steal;
}



unsigned long long pid_cpu_time(int pid) { 

    /*Returns the time the CPU gives to a process (PID) in kernel and user mode.
    It comprises threads in /proc/[PID]/task/[TID]*/
    

    FILE *fp;
    unsigned long long utime, stime;
    
    char stat_file[256];
    sprintf(stat_file, "/proc/%d/stat", pid);
    
    if (!(fp = fopen(stat_file, "r"))) 
    {
                return 0;
    }

    // Skip to the utime field
    fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu", &utime, &stime);
    fclose(fp);
    
    return utime + stime;
}


double get_pid_cpu_usage(int pid, int interval_ms) 
{

    unsigned long long start_process_time = pid_cpu_time(pid);
    unsigned long long start_total_time = total_cpu_time();

    //As in the *stat* files, the time is from the begining of system/process, I use positive numbers in interval_ms to analyze window times.

    if(interval_ms>0)
    {

        //TODO -> PUT OUT OF WHILE and maybe put the while in the main file??
        // Convert the interval in milliseconds to a timespec struct for nanosleep
        struct timespec interval_timespec;
        interval_timespec.tv_sec = interval_ms / 1000;
        interval_timespec.tv_nsec = (interval_ms % 1000) * 1000000;
        
        //SLEEP ORDER
        nanosleep(&interval_timespec, NULL);

        unsigned long long end_process_time = pid_cpu_time(pid);
        unsigned long long end_total_time = total_cpu_time();

        double process_cpu_time_diff = end_process_time - start_process_time;
        double total_cpu_time_diff = end_total_time - start_total_time;
          
        //The usage of a process is relative to the total time that the CPU grants
    
        return (process_cpu_time_diff / total_cpu_time_diff);
    }
    return start_total_time/start_process_time;

}
 
