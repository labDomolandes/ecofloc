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



#include "sd.h"

void init_sd_features(sd_features *features)
{
    FILE* file = fopen(FEATURES_FILE, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open the config file %s\n", FEATURES_FILE);
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#')
            continue;

        if (sscanf(line, "write_power=%f", &(features->write_power)) == 1)
            ; // Successfully parsed write_power
        else if (sscanf(line, "read_power=%f", &(features->read_power)) == 1)
            ; // Successfully parsed read_power
        else if (sscanf(line, "write_max_rate=%ld", &(features->write_max_rate)) == 1)
            ; // Successfully parsed write_max_rate
        else if (sscanf(line, "read_max_rate=%ld", &(features->read_max_rate)) == 1)
            ; // Successfully parsed read_max_rate
    }

    fclose(file);
}

long read_bytes(int pid)
{
    return io_bytes(pid, "read_bytes:");
}

long written_bytes(int pid)
{
    return io_bytes(pid, "write_bytes:");
}

long io_bytes(int pid, const char* key)
{
    char path[256];
    snprintf(path, sizeof(path), PROC_IO_PATH, pid);

    FILE* file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Failed to open file for PID %d. The process may not exist or it was closed.\n", pid);
        //exit(EXIT_FAILURE);
        return 0; //0 bytes
    }

    char line_key[256];
    long bytes = 0;
    int found = 0;

    while (fscanf(file, "%255s %ld", line_key, &bytes) == 2)
    {
        if (strcmp(line_key, key) == 0)
        {
            found = 1;
            break;
        }
    }

    fclose(file);

    if (!found)
    {
        printf("Key '%s' not found for PID %d.\n", key, pid);
        return -1;
    }

    return bytes;
}





