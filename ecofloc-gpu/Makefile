CC=gcc
#CFLAGS=-Wall
LDFLAGS=-lpthread -lrt

TARGET_GPU=ecofloc-gpu.out
SOURCE_GPU=main.c
ADDITIONAL_SOURCES=gpu.c pid_energy.c results_map.c comm_energy.c

all: $(TARGET_GPU)

$(TARGET_GPU): $(SOURCE_GPU) $(ADDITIONAL_SOURCES)
	$(CC) $(CFLAGS) $(SOURCE_GPU) $(ADDITIONAL_SOURCES) $(LDFLAGS) -o $(TARGET_GPU)

clean:
	rm -f $(TARGET_GPU)
