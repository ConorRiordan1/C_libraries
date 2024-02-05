#include "example.h"



queue_root my_queue;
pthread_mutex_t queue_mutex;


tpool_t * tm;

void read_from_socket(void * arg)
{   

        socket_info * descriptors = (socket_info *) arg;

        SSL * ssl_descriptor = descriptors->ssl_socket;

        struct pollfd poll_descriptor = descriptors->file_descriptor;

        char buffer[64];
        int bytes_recv = SSL_read(ssl_descriptor, buffer, sizeof(buffer));

        if (bytes_recv > 0) {
            buffer[bytes_recv] = '\0'; // Null-terminate the received data
            printf("I got: %s\n", buffer);
        } else {
            // Handle error or connection closure
            printf("Error in SSL_read\n");
            close(poll_descriptor.fd);
            //descriptors->fds[descriptors->index] = descriptors->fds[--descriptors->num_file_desc];
            goto EXIT;
        }
        goto EXIT;

    EXIT:
        free(arg);
        return;
}


int push_sockets(socket_info * socket_desc) // old main
{

    //printf(" I am push socket, I got this %d\n",socket);
    printf("\nin example\n");

    tpool_add_work(tm,read_from_socket,socket_desc);




    
    //here
    tpool_wait(tm);
    
    //tpool_destroy(tm);
    //pthread_mutex_destroy(&queue_mutex);
    printf("done\n");

    return 0;











}

int init_my_stuff()
{
    //init_queue(&my_queue);
    tm = tpool_create(3);

    ///pthread_mutex_init(&queue_mutex,NULL);

    return 0;

}