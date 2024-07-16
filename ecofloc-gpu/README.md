# GPU Energy Performance Tool (`ecofloc-gpu`) Installation and Usage

## System Requirements


Before installing `ecofloc-gpu`, ensure your computer is equipped with an **NVIDIA** video card and that nvidia-smi is installed on your system.


- **For Arch-based distributions:**
  - Run as root `# pacman -S nvidia-smi`

- **For Debian-based distributions:**
  - Run as root `# apt-get install nvidia-smi`


## Installation

To install the `ecofloc-gpu` application on your computer, follow these steps:

1. **Compile the Source Code:**
   - Open your terminal.
   - Navigate to the directory containing the `ecofloc-gpu` source code.
   - Execute the command `make` to compile the application.

2. **Uninstallation:**
   - To uninstall, run `make clean` in the same directory. This will remove compiled and intermediate binaries.

## Usage

To run the `ecofloc-gpu` program, you need to execute it as root with the following parameters:

- **Options:**
  1. `-p`: Specify the Process ID (PID) of the process you want to analyze. Alternatively, you can use `-n` to specify the name of the application to measure.
  2. `-t`: Define the total duration of the analysis period.
  3. `-i`: Set the measurement interval (i.e., how frequently the application will measure power consumption).
  4. `-d`: Enable Dynamic Mode, allowing the tool to evaluate applications that can be closed and reopened during the analysis.
  5. `-f`: Export the measurement results to a CSV file specified in `gpu-settings.conf`.
  
  Example Command as root:

  `# ./ecofloc-gpu.out -p [PID or App Name] -i [interval in milliseconds] -t [total time in seconds] --d --f`

## Important Considerations

- If the `ecofloc-gpu` tool reports a power/energy consumption result of `0`, this could indicate that the measurement interval and/or the total analysis time are too short. In such cases, try incrementing the measurement interval and extending the total analysis time.

- If an error about the video card's global power consumption appears, your video card may not be supported, rendering ecofloc-gpu incompatible with your system.

