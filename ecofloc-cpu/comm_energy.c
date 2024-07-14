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


#include"comm_energy.h"

int new_pids[MAX_PIDS];
int treated_pids[MAX_PIDS];
int new_pid_count = 0;
int treated_pid_count = 0;
double total_energy = 0.0;
pthread_mutex_t mutex;
pthread_mutex_t energy_mutex;
pthread_mutex_t treated_mutex;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
int interval_ms_global;
int timeout_s_global;
volatile int active_thread_count = 0;
unsigned long long global_start_time;
//int dynamic_mode = 0;

int is_pid_folder(const char *name) 
{
    while (*name) 
    {
        if (*name < '0' || *name > '9') 
            return 0;
        name++;
    }
    return 1;
}

int get_cmd_from_pid(char *pid, char *buffer, size_t buffer_size) 
{
    char path[512];
    sprintf(path, "/proc/%s/comm", pid);
    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        if (fgets(buffer, buffer_size, fp) == NULL) {
            fclose(fp);
            return 0;
        }
        fclose(fp);
        buffer[strcspn(buffer, "\n")] = 0;
        return 1;
    }
    return 0;
}


void *pid_energy_thread(void *arg) 
{
    
    /*
    * TODO -> Verify this and if this causes problems: 
    * When a new process is created, It should be analyzed for the remaining global time.
    * -> unsigned long long timeout_s_local = timeout_s_global - (time(NULL) - global_start_time);
    * However, app is killed finished when discover_pids finishes
    */ 

    int pid = *((int *)arg);
    free(arg);

    pthread_mutex_lock(&energy_mutex);
    active_thread_count++;
    pthread_mutex_unlock(&energy_mutex);

    double energy = pid_energy(pid, interval_ms_global, timeout_s_global);

    pthread_mutex_lock(&energy_mutex);
    total_energy += energy;
    active_thread_count--;
    if (active_thread_count == 0) {
        pthread_cond_signal(&cond_var);
    }
    pthread_mutex_unlock(&energy_mutex);

    return NULL;
}

void launch_energy_threads() 
{
    pthread_mutex_lock(&mutex);
    while (new_pid_count > 0) 
    {
        for (int i = 0; i < new_pid_count; i++) 
        {
            if (!is_treated(new_pids[i])) 
            {
                int *pid = malloc(sizeof(int));
                *pid = new_pids[i];
                pthread_t tid;
                pthread_create(&tid, NULL, pid_energy_thread, pid);
                pthread_detach(tid);
                treated_pids[treated_pid_count++] = new_pids[i];
            }
        }
        new_pid_count = 0;
    }
    pthread_mutex_unlock(&mutex);
}

int is_treated(int pid) 
{
    pthread_mutex_lock(&treated_mutex);
    for (int i = 0; i < treated_pid_count; i++) 
    {
        if (treated_pids[i] == pid) 
        {
            pthread_mutex_unlock(&treated_mutex);
            return 1;  // PID has been treated
        }
    }
    pthread_mutex_unlock(&treated_mutex);
    return 0;  // PID has not been treated
}

void *discover_pids(void *arg) 
{
    const char *cmd_name = (const char *)arg;
    global_start_time = time(NULL);

    while (1) 
    {
        int found_new_pid = 0;
        DIR *dir = opendir("/proc");
        if (dir == NULL) 
        {
            perror("Failed to open /proc");
            exit(EXIT_FAILURE);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) 
        {
            if (entry->d_type == DT_DIR && is_pid_folder(entry->d_name)) 
            {
                char cmd[256];
                if (get_cmd_from_pid(entry->d_name, cmd, sizeof(cmd)) && strcmp(cmd, cmd_name) == 0) 
                {
                    int pid = atoi(entry->d_name);
                    if (!is_treated(pid)) 
                    {
                        pthread_mutex_lock(&mutex);
                        new_pids[new_pid_count++] = pid;
                        found_new_pid = 1;
                        pthread_mutex_unlock(&mutex);
                    }
                }
            }
        }
        closedir(dir);

        if (found_new_pid) 
            launch_energy_threads();



       signal(SIGINT, handle_sigint); //CNTRL + C from pid_energy.h

        if(keep_running == 0) //CNTRL + C declared in the pid_energy.h file
          break;
       if(((time(NULL) - global_start_time) >= timeout_s_global)) //Timeout
          break;
       if (!found_new_pid && active_thread_count == 0 && dynamic_mode==0) //No more pids and no dynamic_mode         
          break;
       


        sleep(SLEEP_PID_DISCOVERING);
    }
    return NULL;
}

void comm_energy(const char *cmd_name, int interval_ms, int timeout_s) 
{
    interval_ms_global = interval_ms;
    
    if(timeout_s>=0)
        timeout_s_global = timeout_s;
    else 
        timeout_s_global = INT_MAX;


    pthread_t discovery_tid;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&energy_mutex, NULL);
    pthread_mutex_init(&treated_mutex, NULL);

    pthread_create(&discovery_tid, NULL, discover_pids, (void *)cmd_name);
    pthread_join(discovery_tid, NULL);  // Wait for discovery thread to finish

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&energy_mutex);
    pthread_mutex_destroy(&treated_mutex);

}
