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


#include "results_map.h"
#include "pid_energy.h"
#include "comm_energy.h"

int export_to_csv = 0; //Extern variable in results_map.h
int dynamic_mode = 0; //Extern in comm_energy.h


int main(int argc, char **argv)
{
    int pid = 0;
    char *processName = NULL;
    double interval_ms = 0.0;
    double total_time_s = 0.0;

    export_to_csv = 0;
    dynamic_mode = 0;

    int opt;
    while ((opt = getopt(argc, argv, "p:n:i:t:fd")) != -1)
    {
        switch (opt)
        {
            case 'f':
                export_to_csv = 1;  // Set export to CSV flag
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
            case 'd':
                dynamic_mode = 1;  // Set dynamic mode flag
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-p PID] [-n ProcessName] -i INTERVAL_MS -t TOTAL_TIME_S [-f] [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if ((pid == 0 && processName == NULL) || interval_ms == 0.0 || total_time_s == 0.0)
    {
        fprintf(stderr, "Usage: %s [-p PID] [-n ProcessName] -i INTERVAL_MS -t TOTAL_TIME_S [-f] [-d]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (pid != 0)
    {
        initialize_results_object(&pid, 1);  //defined in results_map.h
        pid_energy(pid, (int)interval_ms, (int)total_time_s);
    }
    else if (processName != NULL)
    {
        initialize_results_object(processName, 0);  //defined in results_map.h
        comm_energy(processName, (int)interval_ms, (int)total_time_s);
    }

    print_results();
    close_results_object();  

    return 0;
}
