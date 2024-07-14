/*Licensed to the Apache Software Foundation (ASF) under one
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ECOFLOC_DIR "/opt/ecofloc/"

int main(int argc, char *argv[])
{
    if (argc < 2) //if no arguments
    {
        printf("Usage: %s --cpu|--sd|--ram|--nic [options]\n", argv[0]);
        return 1;
    }

    char *app;
    char *args[argc + 1];  // +1 to ensure space for NULL termination

    if (strcmp(argv[1], "--cpu") == 0 || strcmp(argv[1], "--ram") == 0 ||
        strcmp(argv[1], "--sd") == 0 || strcmp(argv[1], "--nic") == 0 
        || strcmp(argv[1], "--gpu") == 0)
    {
        if (argc < 8)
        {
            printf("Usage for CPU/RAM/SD/NIC: %s %s [ -p [PID] or -n [ProcessName] ] -i [interval in milliseconds] -t [timeout in seconds] [-d [true|false]] [-f [filename]]\n", argv[0], argv[1]);
            return 1;
        }

        char fullPath[256]; 

        if (strcmp(argv[1], "--cpu") == 0)
            snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, "ecofloc-cpu.out");
        else if (strcmp(argv[1], "--ram") == 0)
            snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, "ecofloc-ram.out");
        else if (strcmp(argv[1], "--sd") == 0)
            snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, "ecofloc-sd.out");
        else if (strcmp(argv[1], "--nic") == 0)
            snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, "ecofloc-nic.out");
        else if (strcmp(argv[1], "--gpu") == 0)
            snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, "ecofloc-gpu.out");

        app = fullPath;
        args[0] = app;

        for (int i = 2; i < argc; i++)
            args[i - 1] = argv[i];

        args[argc - 1] = NULL; // NULL-terminate the arguments.
    }
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        return 1;
    }

    if (execvp(app, args) == -1)
    {
        perror("Error executing the application");
        return 1;
    }

    return 0; // This point is never reached.
}