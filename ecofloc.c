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
    if (argc < 2)
    {
        printf("Usage:\n\n");
        printf("  # Monitor a specific process (CPU, RAM, SD, NIC)\n");
        printf("  ecofloc --cpu|--ram|--sd|--nic -p <PID> | -n <ProcessName> \\\n");
        printf("          -i <interval_ms> -t <timeout_s> [-d true|false] [-f <output_file>]\n\n");
        printf("  # Monitor the entire system (CPU, RAM, SD, NIC)\n");
        printf("  ecofloc --cpu|--ram|--sd|--nic --system \\\n");
        printf("          -i <interval_ms> -t <timeout_s> [-d true|false] [-f <output_file>]\n");
        return 1;
    }

    const char *subtool = NULL;

    if (strcmp(argv[1], "--cpu") == 0)
        subtool = "ecofloc-cpu.out";
    else if (strcmp(argv[1], "--ram") == 0)
        subtool = "ecofloc-ram.out";
    else if (strcmp(argv[1], "--sd") == 0)
        subtool = "ecofloc-sd.out";
    else if (strcmp(argv[1], "--nic") == 0)
        subtool = "ecofloc-nic.out";
    else if (strcmp(argv[1], "--gpu") == 0)
        subtool = "ecofloc-gpu.out";
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        return 1;
    }

    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "%s%s", ECOFLOC_DIR, subtool);

    // Build args for execvp
    char *args[argc];  // argc elements: tool + (argc - 2) args + NULL
    args[0] = fullPath;

    for (int i = 2; i < argc; i++)
        args[i - 1] = argv[i];

    args[argc - 1] = NULL;

    execvp(fullPath, args);

    perror("Error executing the application");
    return 1;
}
