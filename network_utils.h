#ifndef __NETWORK_UTILS__
#define __NETWORK_UTILS__
#include "example.h"
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <poll.h>


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>


extern struct pollfd fds[MAX_CLIENTS + 1];
extern SSL_CTX * ctx;
int server();
int finish_it();

typedef struct client
{
    struct pollfd * client_socket;
    SSL * client_ssl;
}client;

#endif