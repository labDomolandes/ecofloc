# Storage Device Energy Performance Tool (`ecofloc-sd`) Installation and Usage

## Installation

To install the `ecofloc-sd` application on your computer, follow these steps:

1. **Compile the Source Code:**
   - Open your terminal.
   - Navigate to the directory containing the `ecofloc-sd` source code.
   - Execute the command `make` to compile the application.

2. **Uninstallation:**
   - To uninstall, run `make clean` in the same directory. This will remove compiled and intermediate binaries.

## Usage

To run the `ecofloc-sd` program, you need to execute it as root with the following parameters:

- **Execute as Root:**
  - Run the program using `sudo ./ecofloc-sd.out` with the necessary options.

- **Options:**
  1. `-p`: Specify the Process ID (PID) of the process you want to analyze. Alternatively, you can use `-n` to specify the name of the application to measure.
  2. `-t`: Define the total duration of the analysis period.
  3. `-i`: Set the measurement interval (i.e., how frequently the application will measure power consumption).
  4. `-d`: Enable Dynamic Mode, allowing the tool to evaluate applications that can be closed and reopened during the analysis.
  5. `-f`: Export the measurement results to a specified CSV file.
  

  Example Command:

  `# sudo ./ecofloc-sd.out -p [PID or App Name] -i [interval in milliseconds] -t [total time in seconds] --d --f`


## Important Considerations

- If the `ecofloc-sd` tool reports a power consumption result of `0`, this could indicate that the measurement interval and/or the total analysis time are too short. In such cases, try incrementing the measurement interval and extending the total analysis time.

- If you encounter an error related to "/proc/<PID>/io", ensure you are running the application on a bare-metal machine and not in a virtual environment.

