CC = gcc
CFLAGS = -Isrc/helpers -Iinclude -Wall -Wextra -g -std=gnu99
BIN_DIR = bin
OUT_SERVER = $(BIN_DIR)/stars

# Source files
SRC_SERVER = main.c network_utils.c linked_list.c  processing.c

all: $(OUT_SERVER)

$(OUT_SERVER): $(SRC_SERVER)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(OUT_SERVER) $(SRC_SERVER) -lpthread -lssl -lcrypto -lm

program: $(OUT_SERVER)

clean:
	rm -rf $(BIN_DIR)
	rm -f cert.pem key.pem

.PHONY:test
test:
	./test/tests.py

.PHONY:docs
docs:
	doxygen Doxyfile
	cd latex && make
	rm -rf latex/html latex/rtf latex/xml latex/man latex/rtf latex/tex latex/txt latex/Makefile latex/*.sty latex/*.tex
	mv latex/refman.pdf ./docs
	rm -rf html
	rm -rf latex

setup: generate_certificate

generate_certificate:
run: $(OUT_SERVER)
	$(OUT_SERVER)

generate_certificate:
	openssl req -x509 -newkey rsa:4096 -nodes -keyout bin/key.pem -out bin/cert.pem -days 365

run: $(OUT_SERVER)
	$(OUT_SERVER)
