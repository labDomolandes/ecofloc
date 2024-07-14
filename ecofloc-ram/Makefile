CC=gcc
#CFLAGS=-Wall
LDFLAGS=-lpthread -lrt

TARGET_RAM=ecofloc-ram.out
SOURCE_RAM=main.c
ADDITIONAL_SOURCES= pid_energy.c comm_energy.c results_map.c

all: $(TARGET_RAM)

$(TARGET_RAM): $(SOURCE_RAM) $(ADDITIONAL_SOURCES)
	$(CC) $(CFLAGS) $(SOURCE_RAM) $(ADDITIONAL_SOURCES) $(LDFLAGS) -o $(TARGET_RAM)

clean:
	rm -f $(TARGET_RAM)
