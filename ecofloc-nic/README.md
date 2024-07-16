# Network Interface Controller Energy Performance Tool (`ecofloc-nic`) Installation and Usage

## System Requirements

Before using `ecofloc-nic`, ensure you have `nethogs` installed on your system, version greater than `0.8.7`:

- **For Arch-based distributions:**
  - Install `nethogs` by running as root:
    - `sudo pacman -S nethogs`

- **For Debian-based distributions:**
  - First, update your package list as root:
    - `# apt-get update`
  - Install as root `nethogs` (ensure you get a version > 0.8.7, you might need to check if the latest version is available in your distribution's repository or consider installing from source if the version is too old):
    - `# apt-get install nethogs`
    
## Installation

To install the `ecofloc-nic` application on your computer, follow these steps:

1. **Compile the Source Code:**
   - Open your terminal.
   - Navigate to the directory containing the `ecofloc-nic` source code.
   - Execute the command `make` to compile the application.

2. **Uninstallation:**
   - To uninstall, run `make clean` in the same directory. This will remove compiled and intermediate binaries.

## Usage

To run the `ecofloc-nic` program, you need to execute it as root with the following parameters:

- **Options:**
  1. `-p`: Specify the Process ID (PID) of the process you want to analyze. Alternatively, you can use `-n` to specify the name of the application to measure.
  2. `-t`: Define the total duration of the analysis period.
  3. `-i`: Set the measurement interval (i.e., how frequently the application will measure network performance).
  4. `-d`: Enable Dynamic Mode, allowing the tool to evaluate network applications that can be closed and reopened during the analysis.
  5. `-f`: Export the measurement results to a specified CSV file.
  

  Example Command:

  `# sudo ./ecofloc-nic.out -p [PID or App Name] -i [interval in milliseconds] -t [total time in seconds] --d --f`


## Important Considerations

- If the `ecofloc-nic` tool reports a power consumption result of `0`, this could indicate that the measurement interval and/or the total analysis time are too short. In such cases, try incrementing the measurement interval and extending the total analysis time.
