CC=gcc
CFLAGS=-Wall -lpthread -lrt -w
ADDITIONAL_LIBRARIES = nic.c pid_energy.c comm_energy.c results_map.c

TARGET_NIC=ecofloc-nic.out
SOURCE_NIC=main.c

all: $(TARGET_NIC)

nic: $(TARGET_NIC)

$(TARGET_NIC): $(SOURCE_NIC)
	$(CC) $(CFLAGS) $(SOURCE_NIC) $(ADDITIONAL_LIBRARIES) -o $(TARGET_NIC)

clean:
	rm -f $(TARGET_NIC)

