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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "results_map.h"
#include "pid_energy.h"
#include "system_energy.h"
#include "comm_energy.h"
#include "cpu_map.h"

int export_to_csv = 0; // Extern variable in results_map.h
int dynamic_mode = 0; // Extern in comm_energy.h
char* filePath = NULL; // Extern variable in results_map.h

int main(int argc, char **argv)
{
    int pid = 0;
    char *processName = NULL;
    char *launchCommandPID = NULL;
    char *launchCommandName = NULL;
    double interval_ms = 0.0;
    double total_time_s = 0.0;
    int verbose = 0;
    int system_mode = 0;

    export_to_csv = 0;  // Default no export

    int opt;
    while ((opt = getopt(argc, argv, "p:n:i:t:f:l:L:Sdv")) != -1)
    {
        switch (opt)
        {
            case 'f':
                export_to_csv = 1;
                if (optarg) 
                {
                    filePath = (char *)malloc(1024 * sizeof(char));
                    if (!filePath) 
                    {
                        perror("Memory allocation failed for filePath");
                        exit(EXIT_FAILURE);
                    }
                    strncpy(filePath, optarg, 1023); 
                    filePath[1023] = '\0';  
                } 
                break;
            case 'p':
                pid = atoi(optarg);
                break;
            case 'n':
                processName = optarg;
                break;
            case 'i':
                interval_ms = atof(optarg);
                break;
            case 't':
                total_time_s = atof(optarg);
                break;
            case 'l':
                launchCommandPID = optarg;  
                break;
            case 'L':
                launchCommandName = optarg; 
                break;
            case 'S':
                system_mode = 1;
                break;
            case 'd':
                dynamic_mode = 1;  
                break;
            case 'v':
                verbose = 1;  
                break;
            default:
                fprintf(stderr, "Usage: %s [-p PID] [-n ProcessName] [-l Command] [-L Command] -i INTERVAL_MS -t TOTAL_TIME_S [-S] [-f] [-d] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Count tracking options
    int tracking_options = (pid != 0) + (processName != NULL) + 
                           (launchCommandPID != NULL) + (launchCommandName != NULL) + 
                           (system_mode != 0);

    if (tracking_options != 1)
    {
        fprintf(stderr, "Error: You must specify exactly one of -p, -n, -l, -L, or -S\n");
        exit(EXIT_FAILURE);
    }

    if (launchCommandPID != NULL && launchCommandName != NULL)
    {
        fprintf(stderr, "Error: -l and -L cannot be used together.\n");
        exit(EXIT_FAILURE);
    }

    // Case: -S -> System-wide analysis
    if (system_mode)
    {
        char *sys = "System_Wide";
        init_cpu_features(&cpu_specs);
        initialize_results_object(sys, 3); 
        system_energy((int)interval_ms, (int)total_time_s);
        print_results();
    }

    // Case: -l command -> Launch and analyze by PID
    else if (launchCommandPID != NULL)
    {
        pid_t child_pid = fork();

        if (child_pid == 0)  
        {
            setsid();
            if (!verbose)
                freopen("/dev/null", "w", stderr); 
            
            char *args[] = {launchCommandPID, NULL};
            execvp(launchCommandPID, args);

            perror("Error launching command");
            exit(EXIT_FAILURE);
        }
        else if (child_pid > 0)  
        {
            init_cpu_features(&features);
            int pid_copy = child_pid;
            initialize_results_object(&pid_copy, 1);
            pid_energy(child_pid, (int)interval_ms, (int)total_time_s);
            print_results();
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Case: -L command -> Launch and analyze by process name
    else if (launchCommandName != NULL)
    {
        pid_t child_pid = fork();

        if (child_pid == 0)  
        {
            setsid();
            if (!verbose)
                freopen("/dev/null", "w", stderr);
            
            char *args[] = {launchCommandName, NULL};
            execvp(launchCommandName, args);

            perror("Error launching command");
            exit(EXIT_FAILURE);
        }
        else if (child_pid > 0)  
        {
            init_cpu_features(&features);
            initialize_results_object(launchCommandName, 2);
            comm_energy(launchCommandName, (int)interval_ms, (int)total_time_s);
            print_results();
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Case: -p PID -> Analyze an existing PID
    else if (pid != 0)
    {
        init_cpu_features(&features);
        initialize_results_object(&pid, 1);
        pid_energy(pid, (int)interval_ms, (int)total_time_s);
        print_results();
    }

    // Case: -n processName -> Analyze an existing process by name
    else if (processName != NULL)
    {
        init_cpu_features(&features);
        initialize_results_object(processName, 2);
        comm_energy(processName, (int)interval_ms, (int)total_time_s);
        print_results();
    }

    close_results_object();

    if (filePath) 
    {
        free(filePath);
    }

    return 0;
}