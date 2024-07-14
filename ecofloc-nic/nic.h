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


#ifndef NIC_H
#define NIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEATURES_FILE "/opt/ecofloc/nic_features.conf"

typedef struct 
{
    char iface[32];
    float upload_power;
    float download_power;
    long upload_max_rate;
    long download_max_rate;
} nic_features;


void init_nic_features(nic_features *features);



#endif 
