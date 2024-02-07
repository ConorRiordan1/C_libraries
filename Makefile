# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=thread

# Source files
SRC_CLIENT = client.c
SRC_SERVER = main.c network_utils.c example.c queue.c threadpool.c
# Output files

OUT_SERVER = server

all: $(OUT_SERVER)

$(OUT_SERVER): $(SRC_SERVER)
	$(CC) $(CFLAGS) -o $(OUT_SERVER) $(SRC_SERVER) -lpthread -lssl -lcrypto

clean:
	rm -f  $(OUT_SERVER)
