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
#include "linked_list.h"
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <signal.h>
#include "processing.h"

#define TIMEOUT_LIMIT 5 // timeout in seconds
#define IPV4 4
#define IPV6 6
#define POLL_RETURN_RATE 5000
#define CONN_MAX 258

int server (linkedlist * p_maplist, linkedlist * p_accountlist,int port, int ipv);
int cleanup_sockets();

extern pthread_mutex_t printf_mutex;


#endif

/* END OF FILE*/
