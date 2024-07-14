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


#include"gpu.h"


double gpu_power() 
{
    char buffer[128];
    float power_draw = 0.0;
    FILE *fp;

    fp = popen("nvidia-smi --query-gpu=power.draw --format=csv,noheader,nounits", "r");
    if (fp == NULL) 
    {
        fprintf(stderr, "Failed to run command\n");
        exit(1);
    }

    if (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) 
    {
        sscanf(buffer, "%f", &power_draw);
    }

    pclose(fp);
    return power_draw;
}