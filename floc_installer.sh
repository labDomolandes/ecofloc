 
#!/bin/bash

number_hw_success=0
total_hw_success=5

################################################################################
#############################         TOOLS    #################################
################################################################################

print_intro()
{
    # Main script execution
    printf "\n"
    printf "\n"
    echo "********************************************************************"
    echo "*************FLOC: Energy Measuring System Tool for Linux***********"
    echo "********************************************************************"
    printf "\n"
    echo "Starting FLOC installer..."
    printf "\n"
    printf "\n"
}

print_success() 
{
    local color_reset='\033[0m'
    local color_green='\033[0;32m'
    local color_yellow='\033[1;33m'

    if [ "$number_hw_success" -lt "$total_hw_success" ]; then
        printf "\n"
        echo -e "${color_yellow}FLOC was successfully installed in the /opt/ directory but some of the hardware components were not considered due to a dependency failure.${color_reset}"
        echo -e "${color_green}To uninstall it, please, run make uninstall from the FLOC git folder${color_reset}"
        printf "\n"
    else
        printf "\n"
        echo -e "${color_green}FLOC was SUCCESSFULLY installed in your system in the /opt/ directory${color_reset}"
        echo -e "${color_green}To uninstall it, please, run make uninstall from the FLOC git folder${color_reset}"
        printf "\n"
    fi
    echo -e "${color_green}Please, always run FLOC applications as root${color_reset}"

}

# PRINT STEP'S RESULT
print_status() 
{
    local message=$1
    local status=$2
    local color_reset='\033[0m'
    local color_green='\033[0;32m'
    local color_red='\033[0;31m'
    if [ "$status" -eq 1 ]; then
        echo -e "$message ${color_green}[OK]${color_reset}"
    else
        echo -e "$message ${color_red}[FAIL]${color_reset}"
    fi
}

install_gui() 
{
    return 0
}



################################################################################
############################   CHECK DEPS    ###################################
################################################################################


###########################################

check_root() 
{
    if [ "$EUID" -ne 0 ]; then
        print_status "Checking if running as root. Please run the installer as root." 0
        return 1
    else
        print_status "Checking if running as root" 1
        return 0
    fi
}

###########################################

check_gcc() 
{
    if ! command -v gcc &> /dev/null; then
        print_status "Checking if gcc is installed. Please, install gcc" 0
        return 1
    else
        print_status "Checking if gcc is installed" 1
        return 0
    fi
}

###########################################


check_cpu() 
{
    ######
    # BASE FREQ.
    ######

    local cpu_info_file="/proc/cpuinfo"
    local buf

    # Check if /proc/cpuinfo exists
    if [ ! -f "$cpu_info_file" ]; then
        print_status "CPU -> Checking CPU info file. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
        return 1
    else
        print_status "CPU -> Checking CPU info file" 1
    fi
    #If the freq. number is present in the file
    while IFS= read -r buf; do
        # Look for the line starting with "model name"
        if [[ "$buf" == model\ name* ]]; then
            local frequency_str
            frequency_str=$(echo "$buf" | awk -F '@' '{print $2}')
            if [ -n "$frequency_str" ]; then
                 # If a number is present before GHz"
                 print_status "CPU -> Checking if cpu base freq. is reachable" 1
                 break
                
            else
                print_status "CPU -> Checking if cpu base freq. is reachable. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
                return 1
            fi
        fi
    done < "$cpu_info_file"

    ######
    # CURR. FREQ. of CORE 0. -> If the file exists and if it changes over the time
    ######

    local cpu_freq_file="/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"

    if [ ! -f "$cpu_freq_file" ]; then
        print_status "CPU -> Checking if CPU current frequency is reachable. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
        return 1
    fi

    local initial_freq
    initial_freq=$(cat "$cpu_freq_file")

    local change_freq=0  # Initialize change variable
    local end_time=$((SECONDS + 5))  # Set the end time for 5 seconds later

    while [ $SECONDS -lt $end_time ]; do
        local current_freq
        current_freq=$(cat "$cpu_freq_file")

        if [ "$initial_freq" -ne "$current_freq" ]; then
            # If the value changes, set change flag and break the loop
            change_freq=1
            break
        fi

        sleep 0.5  # Sleep for a short interval before checking again
    done

    if [ $change_freq -eq 0 ]; then
        print_status "CPU -> Checking if CPU current frequency is reachable. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
        return 1
    fi

    print_status "CPU -> Checking if CPU current frequency is reachable" 1

    ######
    # CURR. VOLT. OF CORE 0
    ######

    local rdmsr_cmd="rdmsr -p 0 0x198 -u --bitfield 47:32"
    local initial_value
    local current_value
    local change_voltage=0  # Initialize change variable

    # Execute rdmsr command and capture the initial value
    initial_value=$($rdmsr_cmd 2>/dev/null)

    local end_time=$((SECONDS + 5))  # Set the end time for 3 seconds later (SECONDS is buid in...)

    while [ $SECONDS -lt $end_time ]; do
        current_value=$($rdmsr_cmd 2>/dev/null)

        # Check if rdmsr command was successful
        if [ $? -ne 0 ]; then
            print_status "CPU -> Voltage: Checking if rdmsr works. Please, install msrtools package" 0
            return 1
        fi

        if [ "$initial_value" -ne "$current_value" ]; then
            # If the value changes, set change flag and break the loop
            change_voltage=1
            break
        fi

        sleep 0.5  # Sleep for a short interval before checking again
    done

    if [ $change_voltage -eq 0 ]; then
        print_status "CPU -> Voltage: Checking if rdmsr works as exptected. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
        return 1
    fi

    print_status "CPU -> Checking if CPU current voltage is reachable" 1

    return 0

}




###########################################



check_ram() 
{
    # I execute a loop for a while, Then I verify after the timeout the concerning mem-events returned by perf
    local perf_timeout=3000

    if ! command -v perf &> /dev/null; then
        print_status "RAM -> Checking if perf is installed. Please, insall perf package" 0
        return 1
    else
        print_status "RAM -> Checking if perf is installed" 1
    fi

    # Fork an infinite loop in the background and redirect stderr to /dev/null
    (while :; do :; done) &> /dev/null &
    local loop_pid=$!

    # Capture the perf stat output in a variable
    local perf_output
    perf_output=$(perf stat -e mem-stores,mem-loads -p "$loop_pid" --timeout $perf_timeout 2>&1)
    local perf_status=$?

    kill -9  "$loop_pid"   # Kill the background process
    wait $! 2>/dev/null ## Funny thing to avoid messages -> Kill always show meesages despite the >. I need to wait kill...

    if [ "$perf_status" -eq 0 ]; then
        # Check if mem-stores or mem-loads contain numerical values
        local mem_stores mem_loads
        while IFS= read -r line; do
            if [[ "$line" == *"mem-stores"* ]]; then
                mem_stores=$(echo "$line" | grep -o '[0-9]*')
            elif [[ "$line" == *"mem-loads"* ]]; then
                mem_loads=$(echo "$line" | grep -o '[0-9]*')
            fi
        done <<< "$perf_output"

        if [[ "$mem_stores" =~ ^[0-9]+$ ]] || [[ "$mem_loads" =~ ^[0-9]+$ ]]; then
            print_status "RAM -> Checking if perf works as expected" 1
            return 0
        else
            print_status "RAM -> Checking if perf works as expected. Possible reasons: You are in a virtual env. or the Kernel must be upgraded or you need to install a higher vesion of perf" 0
            return 1
        fi
    else
        print_status "RAM -> Checking if perf works as expected. Possible reasons: You are in a virtual env. or the Kernel must be upgraded or you need to install a higher version of perf" 0
        return 1
    fi
}




###########################################



check_nic() 
{
    local required_version="0.8.7"

    # Check if nethogs is installed
    if ! command -v nethogs &> /dev/null; then
        print_status "NIC -> nethogs is not installed. Please install nethogs version $required_version or higher." 0
        return 1
    fi

    # Get the installed version of nethogs
    local installed_version
    installed_version=$(nethogs -V | grep -oP '(?<=version )[\d.]+')

    # Compare versions
    if [ "$(printf '%s\n' "$required_version" "$installed_version" | sort -V | head -n1)" != "$required_version" ]; then
        print_status "NIC -> nethogs version is less than $required_version. Please install nethogs version $required_version or higher." 0
        return 1
    fi

    print_status "NIC -> nethogs version $installed_version meets the minimum requirement of $required_version." 1
    return 0
}


###########################################



check_gpu() 
{
    # Check if nvidia-smi is installed
    if ! command -v nvidia-smi &> /dev/null; then
        print_status "GPU -> nvidia-smi is not installed. Please install nvidia-smi." 0
        return 1
    fi

    print_status "GPU -> nvidia-smi is installed." 1
    return 0
}



###########################################


check_sd() 
{
    local sd_test_file="/tmp/sd_test_file"
    local duration=3
    # local write_data="This is a test line to write to disk."

    # Function to write to disk
    write_loop() {
        while :; do
            echo "This is a test line to write to disk." | dd of="$sd_test_file" bs=4K oflag=direct conv=notrunc
            sync
        done
    }

    # Start the write loop in the background
    write_loop &> /dev/null &
    local loop_pid=$!
    # echo $loop_pid

    # Wait the loop to write something
    sleep "$duration"

    # Check if func pid exists
    if [ ! -f "/proc/$loop_pid/io" ]; then
        print_status "SD -> /proc/$loop_pid/io does not exist. Please, try to install FLOC again" 0
        kill -9 "$loop_pid"  # Kill the background process
        wait $! 2>/dev/null
        return 1
    fi

    # Read read_bytes and write_bytes from /proc/$loop_pid/io
    local read_bytes write_bytes
    read_bytes=$(awk '/read_bytes/ {print $2; exit}' /proc/$loop_pid/io)
    write_bytes=$(awk '/write_bytes/ {print $2; exit}' /proc/$loop_pid/io)


    # echo "read_bytes: $read_bytes"
    # echo "write_bytes: $write_bytes"

    # Kill the background process
    kill -9 "$loop_pid"
    wait $! 2>/dev/null

    # Check if read_bytes and write_bytes are numerical values (including 0)
    if [[ "$read_bytes" =~ ^[0-9]+$ ]] && [[ "$write_bytes" =~ ^[0-9]+$ ]]; then
        print_status "SD -> Checking information in the /proc file system" 1
        rm -f "$sd_test_file"  # Clean up the test file
        return 0
    else
        print_status "SD -> Checking information in the /proc file system. Possible reasons: You are in a virtual env. or the Kernel must be upgraded" 0
        rm -f "$sd_test_file"  # Clean up the test file
        return 1
    fi
}






################################################################################
###################        MAIN  :)     #########################################
################################################################################


main() {
    print_intro

    check_root
    result_root=$?
    if [ "$result_root" -ne 0 ]; then exit 1 
    fi

    check_gcc
    result_gcc=$?
    if [ "$result_gcc" -ne 0 ]; then exit 1 
    fi

    # Check CPU
    check_cpu
    result_cpu=$?
    if [ "$result_cpu" -eq 0 ]; then 
        ((number_hw_success++))
        make create_ecofloc_folder > /dev/null 
        make uninstall_cpu > /dev/null
        make clean_cpu > /dev/null

        make cpu > /dev/null
        result_make_cpu=$?
        if [ "$result_make_cpu" -ne 0 ]; then
            print_status "Compiling CPU app." 0     
        else   
            print_status "Compiling CPU app." 1
        fi

        make install_cpu > /dev/null
        result_install_cpu=$?
        if [ "$result_install_cpu" -ne 0 ]; then
            print_status "Installing CPU app." 0     
        else   
            print_status "Installing CPU app." 1
        fi
    fi

    # Check RAM
    check_ram
    result_ram=$?
    if [ "$result_ram" -eq 0 ]; then 
        ((number_hw_success++))
        make create_ecofloc_folder > /dev/null 
        make uninstall_ram > /dev/null
        make clean_ram > /dev/null

        make ram > /dev/null
        result_make_ram=$?
        if [ "$result_make_ram" -ne 0 ]; then
            print_status "Compiling RAM app." 0     
        else   
            print_status "Compiling RAM app." 1
        fi

        make install_ram > /dev/null
        result_install_ram=$?
        if [ "$result_install_ram" -ne 0 ]; then
            print_status "Installing RAM app." 0     
        else   
            print_status "Installing RAM app." 1
        fi
    fi

    # Check NIC
    check_nic
    result_nic=$?
    if [ "$result_nic" -eq 0 ]; then 
        ((number_hw_success++))
        make create_ecofloc_folder > /dev/null 
        make uninstall_nic > /dev/null
        make clean_nic > /dev/null

        make nic > /dev/null
        result_make_nic=$?
        if [ "$result_make_nic" -ne 0 ]; then
            print_status "Compiling NIC app." 0     
        else   
            print_status "Compiling NIC app." 1
        fi

        make install_nic > /dev/null
        result_install_nic=$?
        if [ "$result_install_nic" -ne 0 ]; then
            print_status "Installing NIC app." 0     
        else   
            print_status "Installing NIC app." 1
        fi
    fi

    # Check GPU
    check_gpu
    result_gpu=$?
    if [ "$result_gpu" -eq 0 ]; then 
        ((number_hw_success++))
        make create_ecofloc_folder > /dev/null 
        make uninstall_gpu > /dev/null
        make clean_gpu > /dev/null

        make gpu > /dev/null
        result_make_gpu=$?
        if [ "$result_make_gpu" -ne 0 ]; then
            print_status "Compiling GPU app." 0     
        else   
            print_status "Compiling GPU app." 1
        fi

        make install_gpu > /dev/null
        result_install_gpu=$?
        if [ "$result_install_gpu" -ne 0 ]; then
            print_status "Installing GPU app." 0     
        else   
            print_status "Installing GPU app." 1
        fi
    fi


    # Check SD
    check_sd
    result_sd=$?
    if [ "$result_sd" -eq 0 ]; then 
        ((number_hw_success++))
        make create_ecofloc_folder > /dev/null 
        make uninstall_sd > /dev/null
        make clean_sd > /dev/null

        make sd > /dev/null
        result_make_sd=$?
        if [ "$result_make_sd" -ne 0 ]; then
            print_status "Compiling SD app." 0     
        else   
            print_status "Compiling SD app." 1
        fi

        make install_sd > /dev/null
        result_install_sd=$?
        if [ "$result_install_sd" -ne 0 ]; then
            print_status "Installing SD app." 0     
        else   
            print_status "Installing SD app." 1
        fi
    fi

    
    # Do you want the FLOC GUI?
    read -p "Do you want to install the FLOC GUI? [Y/N]: " install_gui_choice
    case "$install_gui_choice" in 
        [Yy]* )
            make install_gui
            print_status "execute the command flocUI as root and follow the instructions :)." 1
            ;;
        [Nn]* )
            echo "Skipping FLOC GUI installation."
            ;;
        * )
            echo "Invalid choice. Skipping FLOC GUI installation."
            ;;
    esac


    print_success


}

main




