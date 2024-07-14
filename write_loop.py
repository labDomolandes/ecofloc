import os
import time

def write_loop(file_path, duration):
    write_data = "This is a test line to write to disk.\n"
    start_time = time.time()
    with open(file_path, 'a') as f:
        while time.time() - start_time < duration:
            f.write(write_data)
            f.flush()
            os.fsync(f.fileno())

if __name__ == "__main__":
    write_loop("/tmp/sd_test_file", 60)
