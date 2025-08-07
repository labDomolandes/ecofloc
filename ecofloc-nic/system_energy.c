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


#include "system_energy.h"
#include "results_map.h"


nic_features n_features;


volatile sig_atomic_t stay_running = 1;

void handle_signal(int sig) 
{
    stay_running = 0;
}

double system_energy(int interval_ms, int timeout_s)
{
    time_t start_time, current_time;
    double total_energy = 0.0;
    char command[512];
    char output[1024];

    signal(SIGINT, handle_signal);

    start_time = time(NULL);



    // 68 years :)
    if (timeout_s < 0) timeout_s = INT_MAX;

    // All to milliseconds to get the iterations to perform
    int total_iterations = (int)(timeout_s * 1000.0 / interval_ms);

    /*
    * PATCH: Instead of stopping the loop based on timeout expiration, EcoFloc now iterates 
    * based on the computed number of iterations. 
    * This approach prevents inconsistencies when handling multiple measurements. 
    * Example: If collecting system-wide stats takes longer than the specified interval, 
    * relying solely on elapsed time could cause an early exit before completing the intended cycles.
    */

    int iteration = 1;
    while (stay_running && iteration <= total_iterations)
    {
        /*
         * Construct and run the command to gather total system network usage.
         * We use nethogs in default mode to report per-process usage, then we sum it up.
         * This provides a rough estimate of total upload and download rates across the system.
         */
        sprintf(command, "timeout %d nethogs %s -t -v 0 -d 1 | awk 'NF>=2 {print $(NF-1), $NF}'",
            interval_ms / 1000, n_features.iface);


        FILE *fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("Failed to run Nethogs");
            break;
        }

        while (fgets(output, sizeof(output) - 1, fp) != NULL)
        {
            double upload_rate, download_rate;
            if (sscanf(output, "%lf %lf", &upload_rate, &download_rate) != 2)
            {
                continue;
            }

            // printf("ITERATION->%d\n",iteration);
            // printf("upload->%f\n",upload_rate);
            // printf("download->%f\n",download_rate);

            double upload_power = n_features.upload_power * (upload_rate / n_features.upload_max_rate);
            double download_power = n_features.download_power * (download_rate / n_features.download_max_rate);

            //  printf("DL POWER->%f\n",n_features.download_power);
            // printf("UP POWER->%f\n",n_features.upload_power);
            // printf("upload RATE MAX->%ld\n",n_features.upload_max_rate);
            // printf("download RATE MAX->%ld\n",n_features.download_max_rate);



            double avg_interval_power = upload_power + download_power;

            double interval_seconds = (double) interval_ms / 1000.0;
            double interval_energy = avg_interval_power * interval_seconds; // Energy in joules for the interval

            write_results(-1, time(NULL) - start_time, avg_interval_power, interval_energy, iteration, interval_ms);

            total_energy += interval_energy;
        }

        pclose(fp);
        current_time = time(NULL);
        iteration++;
    }

    return total_energy; // Return total energy in joules
}
