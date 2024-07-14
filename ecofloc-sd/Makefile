CC=gcc
CFLAGS= -DDEFAULT_CONFIG_PATH=\"/opt/ecofloc/sdFeatures.conf\" -lpthread -lrt -w
ADDITIONAL_LIBRARIES = pid_energy.c sd.c results_map.c comm_energy.c

TARGET_SD=ecofloc-sd.out 
SOURCE_SD=main.c

all: $(TARGET_SD)

sd: $(TARGET_SD)

$(TARGET_SD): $(SOURCE_SD)
	$(CC) $(CFLAGS) $(SOURCE_SD) $(ADDITIONAL_LIBRARIES) -o $(TARGET_SD)

clean:
	rm -f $(TARGET_SD)

