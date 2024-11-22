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

char* SHARED_OBJ_NAME = NULL; 


static pthread_mutex_t pid_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t comm_mutex = PTHREAD_MUTEX_INITIALIZER;

int create_results_object(const char* name, int* fd, void** ptr)
{
    /*
    *Create shared memory space
    */

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
    
    /*
    *Set size and permissions: write + visible for different processes.  
    */

    *ptr = mmap(0, SHARED_OBJ_SIZE, PROT_WRITE, MAP_SHARED, *fd, 0);
    if (*ptr == MAP_FAILED)
    {
        perror("Mapping failed");
        close(*fd);
        *ptr = NULL;
        return -1;
    }

    memset(*ptr, 0, SHARED_OBJ_SIZE);


    if(export_to_csv==1)
    {
        FILE *configFile = fopen(CONFIG_PATH, "r");
        if (configFile) 
        {
            char filePath[1024];
            if (fgets(filePath, sizeof(filePath), configFile)) 
            {
                /*
                * Remove any newline character at the end of filePath...to have a "clean" path
                */ 
                filePath[strcspn(filePath, "\n")] = 0;

                char fullFilePath[1024]; // Concatenate SHARED_OBJ_NAME and ".csv" to filePath
                snprintf(fullFilePath, sizeof(fullFilePath), "%s%s.csv", filePath, SHARED_OBJ_NAME);
                export_file = fopen(fullFilePath, "a");

                if (!export_file) 
                    perror("Failed to open export file specified in settings.conf");
            } 
            else 
            {
                fprintf(stderr, "Failed to read the first line from settings.conf\n");
            }
            fclose(configFile);
        } 
        else 
        {
            perror("Failed to open settings.conf");
        }
    }

    return 0;
}

void initialize_results_object(void *identifier, int is_pid)
{

    pthread_mutex_lock(&pid_mutex);
    
    SHARED_OBJ_NAME = (char*) malloc(200); //for long commands' name
    if (SHARED_OBJ_NAME == NULL) //if error
    {
        pthread_mutex_unlock(&pid_mutex);
        return;
    }

    /*
    * The memory starting at pid_ptr should be treated as a results object. 
    * Any time we update global_results, we are directly updating the shared memory block
    */
    
    if (is_pid) 
    {
        sprintf(SHARED_OBJ_NAME, "%s%s%d", SHARED_OBJ_NAME_ROOT, "PID_" , *((int*) identifier));
        create_results_object(SHARED_OBJ_NAME, &pid_fd, &pid_ptr);

        global_results = (results*) pid_ptr;
        global_results->identifier.pid = *((int*) identifier);
        global_results->is_pid = 1;
        global_results->elapsed_time=0;
        global_results->count=0;
    } 
    else 
    {

        sprintf(SHARED_OBJ_NAME, "%s%s%s", SHARED_OBJ_NAME_ROOT, "COMM_", (char*) identifier);
        create_results_object(SHARED_OBJ_NAME, &pid_fd, &comm_ptr);

        global_results = (results*) comm_ptr;
        strncpy(global_results->identifier.comm_name, (char*) identifier, 255);
        global_results->identifier.comm_name[255] = '\0'; // Ensure null-termination
        global_results->is_pid = 0;
        global_results->elapsed_time=0;
        global_results->count=0;
    }


    pthread_mutex_unlock(&pid_mutex);
}


void write_results(int pid, int timestamp, double power, double energy)
{

    pthread_mutex_lock(&pid_mutex);

    if (global_results == NULL)
    {
        perror("PID data pointer is null");
        pthread_mutex_unlock(&pid_mutex);
        return;
    }
    
    global_results->elapsed_time = timestamp;
    global_results->total_energy += energy;
    global_results->count++;  // Tracking the accesses number
    

    if (global_results->elapsed_time > 0) 
    {
        /*
        *In case of considering the real elapsed time instead of the measurements quantity
        * global_results->average_power = global_results->total_energy / global_results->elapsed_time;
        */  
        global_results->average_power = global_results->total_energy / global_results->count;
    }

    if (export_to_csv) 
    {
        time_t now;
        struct tm *local;
        char time_str[100];
        time ( &now );
        local = localtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);

        fprintf(export_file, "%s,%d,%.2f,%.2f\n", time_str, pid, power, energy);
        fflush(export_file);
    }  

    //printf("%f %d %f\n", global_results->total_energy, global_results->count, global_results->elapsed_time);


    pthread_mutex_unlock(&pid_mutex);
}


void print_results(int is_pid)
{
    pthread_mutex_lock(&pid_mutex);
    
    results *data;

    if (is_pid) data = (results*) pid_ptr;
    else data = (results*) comm_ptr;

    if (data != NULL)
    {
        if (data->is_pid)
        {
            printf("*****************************\n");
            printf("%s\n", SHARED_OBJ_NAME);
            printf("*****************************\n");
        }
        else
        {
            printf("*****************************\n");
            printf("%s\n", SHARED_OBJ_NAME);
            printf("*****************************\n");
        }

        printf("Average Power : %.2f Watts\n", data->average_power);
        printf("Total Energy : %.2f Joules\n", data->total_energy);

        printf("*****************************\n");

        //printf("Elapsed Time : %.2f seconds\n", data->elapsed_time);
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

    if (shm_unlink(SHARED_OBJ_NAME) == -1)
    {
        perror("PID object unlink failed");
    }

    if (export_to_csv)
    {
        fclose(export_file);
        export_file = NULL;
    }

    free(SHARED_OBJ_NAME);

    pthread_mutex_unlock(&pid_mutex);
}
