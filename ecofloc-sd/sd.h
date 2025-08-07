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
    char device[64]; // Storage device name (e.g., "nvme0n1")
} sd_features;


void init_sd_features(sd_features *features);

/*
 * Description: These functions retrieve the number of bytes read or written by a process from the /proc filesystem 
 * since its start.
 * This function can be used to measure byte counts at two different times to assess usage over a period.
 * Documentation: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/filesystems/proc.rst
 */

long read_bytes_pid(int pid);
long written_bytes_pid(int pid);
long io_bytes_pid(int pid, const char* key);

/*
 * Description: These functions retrieve the number of bytes read or written 
 * by a specific block device from system-wide I/O statistics, as reported by `iostat`.
 * The values correspond to cumulative read and write amounts since boot,
 * extracted from /proc/diskstats via iostat.
 * This function can be used to measure byte counts at two different times 
 * to assess global disk usage over a period.
 * Documentation: https://docs.kernel.org/admin-guide/iostats.html
 */

long read_bytes_sys(sd_features *features);
long written_bytes_sys(sd_features *features);
long io_bytes_sys(sd_features *features, int is_read);


#endif // SD_H
