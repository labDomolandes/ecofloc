# CPU Energy Performance Tool (`ecofloc-cpu`) Installation and Usage

## System Requirements

Before installing `ecofloc-cpu`, ensure that you have `rdmsr` installed on your system. This tool is required for reading model-specific registers (MSR) necessary for energy measurement.

- **For Arch-based distributions:**
  - Run as root `# pacman -S msr-tools`

- **For Debian-based distributions:**
  - Run as root `# apt-get install msr-tools`

    
## Installation

To install the `ecofloc-cpu` application on your computer, follow these steps:

1. **Compile the Source Code:**
   - Open your terminal.
   - Navigate to the directory containing the `ecofloc-cpu` source code.
   - Execute the command `make` to compile the application.

2. **Uninstallation:**
   - To uninstall, run `make clean` in the same directory. This will remove compiled and intermediate binaries.

## Usage

To run the `ecofloc-cpu` program, you need to execute it as root with the following parameters:

- **Options:**
  1. `-p`: Specify the Process ID (PID) of the process you want to analyze. Alternatively, you can use `-n` to specify the name of the application to measure.
  2. `-t`: Define the total duration of the analysis period.
  3. `-i`: Set the measurement interval (i.e., how frequently the application will measure power consumption).
  4. `-d`: Enable Dynamic Mode, allowing the tool to evaluate applications that can be closed and reopened during the analysis.
  5. `-f`: Export the measurement results to a CSV file specified in `cpu-settings.conf`.
  
  Example Command as root:

  `# ./ecofloc-cpu.out -p [PID or App Name] -i [interval in milliseconds] -t [total time in seconds] --d --f`

## Important Considerations

- If the `ecofloc-cpu` tool reports a power/energy consumption result of `0`, this could indicate that the measurement interval and/or the total analysis time are too short. In such cases, try incrementing the measurement interval and extending the total analysis time.

- If you get an error related to the non-existence of `/sys/devices/system/cpu/...`, please ensure you are running the OS on a bare metal machine and not in a virtual environment. 