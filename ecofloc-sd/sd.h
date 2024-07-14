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


#ifndef SD_H
#define SD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROC_IO_PATH "/proc/%d/io"
#define FEATURES_FILE "/opt/ecofloc/sd_features.conf"

typedef struct {
    float write_power;
    float read_power;
    long write_max_rate;
    long read_max_rate;
} sd_features;

void init_sd_features(sd_features *features);

/*
 * Description: These functions retrieve the number of bytes read or written by a process from the /proc filesystem 
 * since its start.
 * This function can be used to measure byte counts at two different times to assess usage over a period.
 * Documentation: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/filesystems/proc.rst
 */

long read_bytes(int pid);
long written_bytes(int pid);
long io_bytes(int pid, const char* key);

#endif // SD_H
