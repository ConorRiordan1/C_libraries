/// @file server.h
/// @author conor riordan
/// @brief operations for running server
#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "processing.h"
#include "linked_list.h"
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <signal.h>

#define TIMEOUT_LIMIT 1000 // timeout in seconds
#define IPV4 4
#define IPV6 6
#define POLL_RETURN_RATE 5000

int server (linkedlist * p_maplist, linkedlist * p_accountlist,int port, int ipv);
int cleanup_sockets();

extern pthread_mutex_t printf_mutex;


#define MAX_NAME_SIZE 64 // max string size
/// @brief represents a client
typedef struct client_t
{
    int      socket_fd;
    time_t   last_message_time;
    int      token_bucket;
    int      errors;
    uint32_t session_id;
    account_node_t * client_account;
    pthread_mutex_t mutex;
} client_t;

#endif

/* END OF FILE*/
