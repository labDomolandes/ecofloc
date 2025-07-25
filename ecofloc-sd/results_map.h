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


#ifndef RESULTS_MAP_H
#define RESULTS_MAP_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

#define CONFIG_PATH "/opt/ecofloc/sd_settings.conf"

#define SHARED_OBJ_SIZE 4096 // 4KB
#define SHARED_OBJ_NAME_ROOT "/ECOFLOC_SD_"

extern char* SHARED_OBJ_NAME;
extern char* filePath;



/*
* Description: The results structure stores the current values of an energy measuring interval. 
*              Values are written to a shared memory portion, enabling IPC. 
*/
#pragma pack(push, 1) // for rpc issues: Align structure fields to 1-byte boundaries, disabling padding.
typedef struct
{
    union { //for pid OR command name
        int pid;
        char comm_name[256];
    } identifier;

    int is_pid;
    double average_power;
    double total_energy;
    unsigned long long elapsed_time;
    int count;

} results;
#pragma pack(pop) 


extern results *global_results;
extern int export_to_csv;
extern FILE *export_file;


/*
* Description: This creates the shared memory space for IPC
*/
int create_results_object(const char* name, int* fd, void** ptr);
/*
* Description: This initializes the results structure and calls create_results_object()
*              Identifier: The pid or the command name is_pid: 1 if is pid
*/
void initialize_results_object(void *identifier, int is_pid);
void write_results(int pid, int timestamp, double power,  double energy, int iterations);
void print_results(int is_pid);
void close_results_object();

#endif // RESULTS_MAP_H
