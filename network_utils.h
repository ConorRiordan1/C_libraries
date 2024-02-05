#ifndef __NETWORK_UTILS__
#define __NETWORK_UTILS__

#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define MY_PORT      8000
#define CONN_MAX     8
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

int main();

#endif