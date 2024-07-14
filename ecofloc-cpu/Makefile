CC=gcc
CFLAGS=-Wall -w  
LDFLAGS=-lpthread -lrt  

TARGET_CPU=ecofloc-cpu.out
SOURCE_CPU=main.c
ADDITIONAL_LIBRARIES=cpu_map.c pid_energy.c comm_energy.c usage.c results_map.c 

all: $(TARGET_CPU)

cpu: $(TARGET_CPU)

$(TARGET_CPU): $(SOURCE_CPU) $(ADDITIONAL_LIBRARIES)
	$(CC) $(CFLAGS) $(SOURCE_CPU) $(ADDITIONAL_LIBRARIES) $(LDFLAGS) -o $(TARGET_CPU)

clean:
	rm -f $(TARGET_CPU)
