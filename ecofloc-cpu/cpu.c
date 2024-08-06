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



#include "cpu.h"

void init_cpu_features(cpu_features *features)
{
    FILE* file = fopen(CONFIG_FILE, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open the CPU config file %s\n", CONFIG_FILE);
        exit(EXIT_FAILURE);
    }

    char line[256];

    /*
    * Parsing CPU feature values from the configuration file.
    */

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#')
            continue;
            
        if (sscanf(line, "cpu_freq_tdp=%lf", &(features->cpu_freq_tdp)) == 1)
            ; // Successfully parsed cpu_freq_tdp
        else if (sscanf(line, "cpu_tdp=%lf", &(features->cpu_tdp)) == 1)
            ; // Successfully parsed cpu_tdp
        else if (sscanf(line, "cpu_voltage_tdp=%lf", &(features->cpu_voltage_tdp)) == 1)
            ; // Successfully parsed cpu_voltage_tdp
    }

    fclose(file);
}
