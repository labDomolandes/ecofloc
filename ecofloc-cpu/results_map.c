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


#include "results_map.h"

int pid_fd = -1;
int comm_fd = -1;
void *pid_ptr = NULL;
void *comm_ptr = NULL;
FILE *export_file = NULL;
results *global_results = NULL; 

static pthread_mutex_t pid_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t comm_mutex = PTHREAD_MUTEX_INITIALIZER;

int create_results_object(const char* name, int* fd, void** ptr)
{
    *fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    
    if (*fd == -1)
    {
        perror("Shared object creation failed");
        return -1;
    }
    
    if (ftruncate(*fd, SHARED_OBJ_SIZE) == -1)
    {
        perror("Setting size failed");
        close(*fd);
        return -1;
    }
    
    *ptr = mmap(0, SHARED_OBJ_SIZE, PROT_WRITE, MAP_SHARED, *fd, 0);
    if (*ptr == MAP_FAILED)
    {
        perror("Mapping failed");
        close(*fd);
        *ptr = NULL;
        return -1;
    }

    if(export_to_csv==1)
    {
        FILE *configFile = fopen(CONFIG_PATH, "r");
        if (configFile) 
        {
            char filePath[1024];
            if (fgets(filePath, sizeof(filePath), configFile)) 
            {
                // Remove any newline character at the end of filePath...to have a "clean" path
                filePath[strcspn(filePath, "\n")] = 0;
                // Open the export file using the path from settings.conf
                export_file = fopen(filePath, "a");
                if (!export_file) 
                {
                    perror("Failed to open export file specified in settings.conf");
                }
            } 
            else 
            {
                fprintf(stderr, "Failed to read the first line from settings.conf\n");
            }
            fclose(configFile);
        } else 
        {
            perror("Failed to open settings.conf");
        }
    }

    return 0;
}

void initialize_results_object(void *identifier, int is_pid)
{
    pthread_mutex_lock(&pid_mutex);
    create_results_object(PID_OBJ_NAME, &pid_fd, &pid_ptr);

    global_results = (results*) pid_ptr;
    
    if (is_pid) 
    {
        global_results->identifier.pid = *((int*) identifier);
        global_results->is_pid = 1;
        global_results->elapsed_time=0;
    } 
    else 
    {
        strncpy(global_results->identifier.comm_name, (char*) identifier, 255);
        global_results->identifier.comm_name[255] = '\0'; // Ensure null-termination
        global_results->is_pid = 0;
        global_results->elapsed_time=0;
    }


    pthread_mutex_unlock(&pid_mutex);
}

void write_results(int pid, int time, double power, double energy)
{
    pthread_mutex_lock(&pid_mutex);

    if (global_results == NULL)
    {
        perror("PID data pointer is null");
        pthread_mutex_unlock(&pid_mutex);
        return;
    }
    
    global_results->total_energy += energy;

    /*
    *In case of multithreading: Store the largest elapsed time
    */  
    if(global_results->elapsed_time<time)
        global_results->elapsed_time = time;
    
    //data->count++;  // In case of tracking the accesses number number

    if (global_results->elapsed_time > 0) 
    {
        global_results->average_power = global_results->total_energy / global_results->elapsed_time;
    }

    if (export_to_csv) 
    {
        fprintf(export_file, "%d,%.2f,%.2f\n", pid, power, energy);
    }    
    pthread_mutex_unlock(&pid_mutex);
}



void print_results()
{
    pthread_mutex_lock(&pid_mutex);
    results *data = (results*) pid_ptr;
    if (data != NULL)
    {
        if (data->is_pid)
        {
            printf("******ECO-FLOC -> CPU********\n");
            printf("PID: %d\n", data->identifier.pid);
            printf("*****************************\n");
        }
        else
        {
            printf("******ECO-FLOC -> CPU********\n");
            printf("Comm Name: %s\n", data->identifier.comm_name);
            printf("*****************************\n");
        }

        printf("Average Power (CPU): %.2f Watts\n", data->average_power);
        printf("Total Energy (CPU): %.2f Joules\n", data->total_energy);
        printf("Elapsed Time (CPU): %.2f seconds\n", data->elapsed_time);
        //printf("Total Iterations: %d\n", data->count);
    }
    else
    {
        printf("No data available.\n");
    }
    pthread_mutex_unlock(&pid_mutex);
}


void close_results_object()
{
    pthread_mutex_lock(&pid_mutex);

    if (munmap(pid_ptr, SHARED_OBJ_SIZE) == -1)
    {
        perror("Unmapping failed");

    }

    if (close(pid_fd) == -1)
    {
        perror("Close failed");
    }

    if (shm_unlink(PID_OBJ_NAME) == -1)
    {
        perror("PID object unlink failed");
    }

    if (export_to_csv)
    {
        fclose(export_file);
        export_file = NULL;
    }


    pthread_mutex_unlock(&pid_mutex);
}
