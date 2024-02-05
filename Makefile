# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=thread -g

# Source files
SRC_CLIENT = client.c
SRC_SERVER = example.c queue.c threadpool.c network_utils.c
# Output files

OUT_SERVER = server

all: $(OUT_SERVER)

$(OUT_SERVER): $(SRC_SERVER)
	$(CC) $(CFLAGS) -o $(OUT_SERVER) $(SRC_SERVER) -lpthread -lssl -lcrypto

clean:
	rm -f  $(OUT_SERVER)
