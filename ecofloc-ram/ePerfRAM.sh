
<<lic
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
lic

#!/bin/bash

################################################################################
# Script Name: ePerfRAM.sh
# Description: This script calculates a PID's RAM power consumption.
# From variables: CPU memory write and read operations
# Usage: ePerfRAM.sh -p PID -t timeout millisecond
# Note: The perf events are different for each CPU architecture
# Note: A safe timeout is 100 msecs, otherwise empty values may exist
################################################################################


pid=0
timeout=0
proccessName=0

ram_energy=0
ram_power_AVG=0

list_of_pid_with_match=0

getInput()
{  
  while getopts "t:p:" opt; do
    case ${opt} in
      t )
        timeout=$(($OPTARG * 1000)) 
        ;;
      p )
        pid=$OPTARG
        ;;
      n )

      \? )
        echo "Invalid option: -$OPTARG" >&2
        exit 1
        ;;
      : )
        echo "Option -$OPTARG requires an argument." >&2
        exit 1
        ;;
    esac
  done
}


verifyInput()
{
  if [ $timeout -lt 100 ]; then
    echo "I need a bigger timeout :(..."
    exit
  fi
  if [ ! -e "/proc/$1/stat" ]; then #If the process does not exist -> exit
    echo "Non-existent PID"
    exit
  fi

}

getRAMCons()
  {
  
    #As the perf output is not a value to pipe but a report, it will be returned to the stderr
    #That is 100ms as that is the minimun window time to count read and write events


    #to replace
    rawResults=$(perf stat -e mem-stores,mem-loads -p $pid --timeout=$timeout 2>&1)
    #rawResults=$(perf stat -e mem-stores,mem-loads -p $pid  2>&1)


    #Replace the "," by a "."
    rawResults=$(echo $rawResults | sed 's/,/./g')

    # Use awk to extract the needed information
    mem_stores=$(echo $rawResults | awk '{print $8}'\
        | iconv -f UTF-8 -t US-ASCII//IGNORE 2>/dev/null)

    mem_loads=$(echo $rawResults | awk '{print $10}' \
        | iconv -f UTF-8 -t US-ASCII//IGNORE 2>/dev/null)

    #(opt) perf return the iime in seconds
    #total_time=$(echo $rawResults | awk '{print $12}' \
    #   | iconv -f UTF-8 -t US-ASCII//IGNORE 2>/dev/null)

    #The Background energy consumption (milliseconds)
    #TODO verify if this is related to a process
    ram_bk=$(echo "$timeout * 0.001 * 1.56" | bc 2>/dev/null) 

    #The active energy consumption
    ram_act=$(echo "scale=10; ($mem_loads * 6.6) + ($mem_stores * 8.7)" \
      | bc 2>/dev/null) #nanoJoules
    ram_act=$(echo "scale=10; ($ram_act / 1000000000)" | bc 2>/dev/null) #->Joules

    #The RAM energy and power consumption
    #ram_energy=$(echo "scale=10; $ram_bk + $ram_act" | bc 2>/dev/null)
    ram_energy=$(echo "scale=10; $ram_act" | bc 2>/dev/null)
    ram_power_AVG=$(echo "scale=10; $ram_energy / ($timeout*0.001) " | bc 2>/dev/null)

}

verifyPrintOutput()
{
  #verify if it's a non empty numerical value  
  if [[ ! -z $ram_energy ]] && \
     [[ $ram_energy =~ ^[0-9]*([.][0-9]+)?$ ]] && \
     [[ ! -z $ram_power_AVG ]] && \
     [[ $ram_power_AVG =~ ^[0-9]*([.][0-9]+)?$ ]]; then

        echo "PID : $pid" 
        echo "RAM_MEASURE_DURATION (S) : $timeout"
        echo "AVG_RAM_POWER (W) : $ram_power_AVG"
        echo "ENERGY_RAM (J) : $ram_energy"
  else
        echo "error somewhere"
    fi

}

getPIDwithMatch()
{
  list_of_pid=$(ps -e | awk '{print  $  1 }' | grep -E '[0-9]')



}

main()
{
  getInput "$@"
  verifyInput
  getRAMCons
  verifyPrintOutput
}

main "$@"
