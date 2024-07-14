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


#include "nic.h"

void init_nic_features(nic_features *features)
{
    FILE* file = fopen(FEATURES_FILE, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open the NIC config file %s\n", FEATURES_FILE);
        exit(EXIT_FAILURE);
    }

    char line[256];

    /* 
    * The rate values are in KBps.  
    */

    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#')
            continue;
            
        if (sscanf(line, "iface=%31s", features->iface) == 1)
            ; // Successfully parsed iface
        else if (sscanf(line, "upload_power=%f", &(features->upload_power)) == 1)
            ; // Successfully parsed upload_power
        else if (sscanf(line, "download_power=%f", &(features->download_power)) == 1)
            ; // Successfully parsed download_power
        else if (sscanf(line, "upload_max_rate=%ld", &(features->upload_max_rate)) == 1)
            ; // Successfully parsed upload_max_rate
        else if (sscanf(line, "download_max_rate=%ld", &(features->download_max_rate)) == 1)
            ; // Successfully parsed download_max_rate
    }

    fclose(file);
}

