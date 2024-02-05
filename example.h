#ifndef EXAMPLE_H
#define EXAMPLE_H
#include <openssl/ssl.h>
#include <openssl/err.h>


#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include <unistd.h>
#include <poll.h>

#define MY_PORT      8000
#define CONN_MAX     8
#define MAX_CLIENTS 100
#define SERVER_PORT 4433
#define BUFFER_SIZE 1024
#define SV_SOCK_PATH "/tmp/us_xfr"
#define min(a, b)    (((a) < (b)) ? (a) : (b))



typedef struct account_t
{
    int account_id;
    int balance;
    int num_deposits;
    int num_withdrawals;
} account_t;

typedef struct transaction_t
{
    int32_t account_id;
    int32_t amount;
} __attribute__((packed)) transaction_t;



typedef struct socket_info
{
    SSL * ssl_socket;
    struct pollfd file_descriptor;
    int index;
    int num_file_desc;
    struct pollfd * fds;
}socket_info;


void read_from_queue(void * arg);

int push_sockets(socket_info * socket_desc);
int init_my_stuff();

#endif