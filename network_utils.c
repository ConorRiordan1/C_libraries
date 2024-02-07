
#include "network_utils.h"

int create_socket(int port)
{
    int s = -1;
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }


    if (listen(s, 1) < 0)
    {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}



SSL_CTX * ctx;
int nfds;
struct pollfd fds[MAX_CLIENTS + 1];
SSL * ssl[MAX_CLIENTS + 1] = { NULL };
client my_clients[MAX_CLIENTS +1];


int handle_exisiting_client(int index)
{
    socket_info * my_socket_info = malloc(sizeof(socket_info));

    if(NULL == my_socket_info)
    {
        printf("alloc error");
        return -1;
    }

    my_socket_info->file_descriptor = fds[index];
    my_socket_info->ssl_socket = ssl[index];
    my_socket_info->index = index;
    my_socket_info->num_file_desc = nfds;
    my_socket_info->fds = fds;

    int recieve = push_sockets(my_socket_info);
    fds[index].fd = 0;
    fds[index].events = 0;
    fds[index].revents = 0;
    SSL_free(ssl[index]);
    ssl[index] = NULL;

    return 1;

}

int server()
{
    nfds = 1;
    init_my_stuff();

    /* Ignore broken pipe signals */
    //signal(SIGPIPE, SIG_IGN);
    SSL_load_error_strings();


    ctx = create_context();
    configure_context(ctx);

    int server_fd = create_socket(SERVER_PORT);


    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for ( int idx = 1; idx < MAX_CLIENTS + 1;idx ++)
    {
        memset(&fds[idx],0,sizeof(fds[idx]));
    }

    write(STDOUT_FILENO, "[+] Server is running\n", 23);
    for (;;)
    {
        int ret = poll(fds, nfds, -1);

        if (ret < 0) {
            perror("Poll error");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    // Accept new connection as normal
                    struct sockaddr_in addr;
                    unsigned int len = sizeof(addr);
                    int client = accept(server_fd, (struct sockaddr*)&addr, &len);
                    if (client < 0)
                    {
                        perror("Unable to accept");
                        continue;
                    }
                    printf("New connection\n");



                    SSL * new_ssl = SSL_new(ctx);

                    SSL_set_fd(new_ssl, client);

                    int accept_check = SSL_accept(new_ssl);

                    if (0 <= accept_check)
                    {
                        // send banner message and add to polling FDs
                        const char greeting[] = "Welcome to my Echo Server! (now with non-blocking)\n";
                        SSL_write(new_ssl, greeting, strlen(greeting));

                        // We can now add the new FD

                        for(int idx = 1; idx <CONN_MAX; idx++)
                        {
                            if (fds[idx].fd == 0)
                            {
                                fds[idx].fd = client;
                                fds[idx].events = POLLIN;
                                ssl[idx] = new_ssl;
                                nfds++;
                                break;
                            }
                            else
                            {
                                printf("there has been an error");
                            }
                        }
                    }                            
                    else
                    {
                        printf("There has been an error");
                    }
                } 
                else //already existing client
                {
                    // Handle data from client
                    // normally a loop to handle partial recv would go here, but since we don't know 
                    // how much data to expect, we're ommitting it for now. A program with some
                    // protocol specific header size could instead read the header_size instead
                    // of the BUFFER_SIZE and then make decisions to read more once deserializing
                    // the header.
                    if(1 == handle_exisiting_client(i))

                    {
                        printf("yay I dealt with a client");
                    }
                    else{
                        printf(" I am a failure at my sole purpose in life");
                    }


                }
            }
        }
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}

int finish_it()
{
        //int server_fd = fds[0].fd;
        //close(server_fd);
        printf("i am hersdsade\n");
        for(int idx = 0; idx <MAX_CLIENTS+1;idx++)
        {
            if (fds[idx].fd != 0)
            {
                close(fds[idx].fd);
            }
        }

        for(int idx = 0; idx <MAX_CLIENTS+1;idx++)
        {
            if (ssl[idx] != NULL)
            {
                SSL_free(ssl[idx]);
                printf("freeing emmeory\n");
            }
        }
        tpool_destroy(tm);
        //SSL_CTX_free(ctx);
        exit(1);
}