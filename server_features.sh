#!/bin/bash

OUTPUT_FILE="server_info.txt"

run_command() {
    echo "====================================" >> $OUTPUT_FILE
    echo "$1" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE
    # Run command, suppress errors if command not found
    $2 2>/dev/null >> $OUTPUT_FILE
    echo "" >> $OUTPUT_FILE
}

check_gcc_version() {
    echo "====================================" >> $OUTPUT_FILE
    echo "GCC Version" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE
    gcc --version | head -n 1 >> $OUTPUT_FILE
    echo "" >> $OUTPUT_FILE
}

check_cpu_frequency() {
    echo "====================================" >> $OUTPUT_FILE
    echo "CPU Frequency" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE

    # Base frequency from /proc/cpuinfo
    local cpu_info_file="/proc/cpuinfo"
    if [ -f "$cpu_info_file" ]; then
        grep "model name" $cpu_info_file | head -n 1 | awk -F '@' '{print "Base Frequency: " $2}' >> $OUTPUT_FILE
    fi

    # Current frequency from scaling_cur_freq
    local cpu_freq_file="/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
    if [ -f "$cpu_freq_file" ]; then
        local current_freq=$(cat "$cpu_freq_file")
        echo "Current Frequency: $(echo "scale=2; $current_freq / 1000" | bc) MHz" >> $OUTPUT_FILE
    fi

    echo "" >> $OUTPUT_FILE
}

check_ram_perf() {
    echo "====================================" >> $OUTPUT_FILE
    echo "RAM Performance (perf)" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE

    local perf_timeout=3000

    if command -v perf &> /dev/null; then
        # Fork an infinite loop in the background and redirect stderr to /dev/null
        (while :; do :; done) &> /dev/null &
        local loop_pid=$!

        # Capture the perf stat output in a variable
        local perf_output
        perf_output=$(perf stat -e mem-stores,mem-loads -p "$loop_pid" --timeout $perf_timeout 2>&1)
        local perf_status=$?

        kill -9 "$loop_pid"   # Kill the background process
        wait $! 2>/dev/null # Avoid messages

        if [ "$perf_status" -eq 0 ]; then
            echo "$perf_output" >> $OUTPUT_FILE
        else
            echo "Perf command failed to execute." >> $OUTPUT_FILE
        fi
    else
        echo "Perf tool is not installed." >> $OUTPUT_FILE
    fi

    echo "" >> $OUTPUT_FILE
}

check_nic() {
    echo "====================================" >> $OUTPUT_FILE
    echo "NIC Information (nethogs)" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE

    if command -v nethogs &> /dev/null; then
        nethogs -V >> $OUTPUT_FILE
    else
        echo "nethogs is not installed." >> $OUTPUT_FILE
    fi

    echo "" >> $OUTPUT_FILE
}

check_sd() {
    echo "====================================" >> $OUTPUT_FILE
    echo "SD Card Performance" >> $OUTPUT_FILE
    echo "====================================" >> $OUTPUT_FILE

    local sd_test_file="/tmp/sd_test_file"
    local duration=3

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

    # Wait the loop to write something
    sleep "$duration"

    # Read read_bytes and write_bytes from /proc/$loop_pid/io
    local read_bytes write_bytes
    if [ -f "/proc/$loop_pid/io" ]; then
        read_bytes=$(awk '/read_bytes/ {print $2; exit}' /proc/$loop_pid/io)
        write_bytes=$(awk '/write_bytes/ {print $2; exit}' /proc/$loop_pid/io)
    else
        echo "/proc/$loop_pid/io does not exist." >> $OUTPUT_FILE
        read_bytes="N/A"
        write_bytes="N/A"
    fi

    # Kill the background process
    kill -9 "$loop_pid"
    wait $! 2>/dev/null

    # Output read_bytes and write_bytes
    echo "Read Bytes: $read_bytes" >> $OUTPUT_FILE
    echo "Write Bytes: $write_bytes" >> $OUTPUT_FILE

    rm -f "$sd_test_file"  # Clean up the test file

    echo "" >> $OUTPUT_FILE
}

# Clear or create the output file
> $OUTPUT_FILE
run_command "System Information" "uname -a"
run_command "CPU Information" "lscpu"
run_command "CPU Information - /proc" "cat /proc/cpuinfo"
# Block devices (disks)
run_command "Block Devices" "lsblk"
run_command "PCI Devices" "lspci"
run_command "USB Devices" "lsusb"
run_command "Mounted Filesystems" "df -h"
# DMI (SMBIOS) dump -> MEMORY and other STUFF
run_command "DMI Table Decoder" "dmidecode"
# Check if running in a virtual machine or container
run_command "Virtualization Check" "systemd-detect-virt"
run_command "Container Check" "grep -cw container /proc/1/environ"
check_gcc_version
check_cpu_frequency
check_ram_perf
check_nic
check_sd

echo "All information has been written to $OUTPUT_FILE"
