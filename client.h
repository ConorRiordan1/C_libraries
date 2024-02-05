#ifndef CLIENT_H

#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sysexits.h>
#include <sys/un.h>

#define PORT         8000
#define SV_SOCK_PATH "/tmp/us_xfr"

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define MAX_PACKET_SIZE 300
#define BUFSIZE         1024

typedef struct
{
    int32_t account;
    int32_t amount;
}__attribute__((packed)) transaction;


int main(int argc, char * argv[]);

#endif

// EOF CLIENT_H
