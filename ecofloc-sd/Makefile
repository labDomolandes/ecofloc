CC = gcc
CFLAGS = -DDEFAULT_CONFIG_PATH=\"/opt/ecofloc/sdFeatures.conf\" -w
LDFLAGS = -lpthread -lrt
ADDITIONAL_LIBRARIES = pid_energy.c sd.c results_map.c comm_energy.c

TARGET_SD = ecofloc-sd.out
SOURCE_SD = main.c

all: $(TARGET_SD)

sd: $(TARGET_SD)

$(TARGET_SD): $(SOURCE_SD) $(ADDITIONAL_LIBRARIES)
	$(CC) $(CFLAGS) $(SOURCE_SD) $(ADDITIONAL_LIBRARIES) -o $(TARGET_SD) $(LDFLAGS)

clean:
	rm -f $(TARGET_SD)
