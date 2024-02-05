#ifndef EXAMPLE_H
#define EXAMPLE_H
#include <openssl/ssl.h>
#include <openssl/err.h>
void add_to_queue(void * arg);

int push_sockets(SSL * socket);
int init_my_stuff();

#endif