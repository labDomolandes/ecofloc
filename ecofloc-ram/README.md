# RAM Energy Performance Tool (`ecofloc-ram`) Installation and Usage

## System Requirements

Before using `ecofloc-ram`, make sure to have `iconv` and `perf` installed on your system:

- **For Arch-based distributions:**
  - Install as root `iconv` (usually included with `glibc`) and `perf` by running:
    - `# pacman -Syu glibc`
    - `# pacman -S linux-tools` (or `linux-zen-tools` for the Zen kernel, adjust accordingly for your kernel)

- **For Debian-based distributions:**
  - `iconv` should be pre-installed with the `libc-bin` package. Install `perf` by running:
    - `# apt-get update`
    - `# apt-get install linux-tools-common linux-tools-$(uname -r)`

## Installation

To install the `ecofloc-ram` application on your computer, follow these steps:

1. **Compile the Source Code:**
   - Open your terminal.
   - Navigate to the directory containing the `ecofloc-ram` source code.
   - Execute the command `make` to compile the application.

2. **Uninstallation:**
   - To uninstall, run `make clean` in the same directory. This will remove compiled and intermediate binaries.

## Usage

To run the `ecofloc-ram` program, you need to execute it as root with the following parameters:

- **Execute as Root:**
  - Run the program using `sudo ./ecofloc-ram.out` with the necessary options.

- **Options:**
  1. `-p`: Specify the Process ID (PID) of the process you want to analyze. Alternatively, you can use `-n` to specify the name of the application to measure.
  2. `-t`: Define the total duration of the analysis period.
  3. `-i`: Set the measurement interval (i.e., how frequently the application will measure power consumption).
  4. `-d`: Enable Dynamic Mode, allowing the tool to evaluate applications that can be closed and reopened during the analysis.
  5. `-f`: Export the measurement results to a specified CSV file.
  
  Example Command:

  `# sudo ./ecofloc-ram.out -p [PID or App Name] -i [interval in milliseconds] -t [total time in seconds] --d --f`


## Important Considerations

- If the `ecofloc-ram` tool reports a power consumption result of `0`, this could indicate that the measurement interval and/or the total analysis time are too short. In such cases, try incrementing the measurement interval and extending the total analysis time.
