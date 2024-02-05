
#include "network_utils.h"
int set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

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

int ssl_accept_loop(SSL * new_ssl, int client_fd)
{
    int accept_check = -1;
    while (0 > accept_check)
    {
        accept_check = SSL_accept(new_ssl);
        if (0 > accept_check)
        {
            int err_code = SSL_get_error(new_ssl, accept_check);
            if ((err_code != SSL_ERROR_WANT_ACCEPT) &&
                (err_code != SSL_ERROR_WANT_READ) &&
                (err_code != SSL_ERROR_WANT_WRITE))
            {
                // Handle SSL_accept error
                printf("This is error code %d\n",err_code);
                ERR_print_errors_fp(stderr);
                SSL_free(new_ssl);
                close(client_fd);
                // We DO NOT close/free the SSL_CTX since it's reused for every new fd
                break;
            }
        }
    }

    return accept_check;
}

int ssl_read_all(SSL * ssl_socket, char * buffer, int limit)
{
    int bytes_recv = -1;
    while (0 >= bytes_recv)
    {
        bytes_recv = SSL_read(ssl_socket, buffer, limit);
        if (0 > bytes_recv)
        {
            int ssl_err = SSL_get_error(ssl_socket, bytes_recv);
            // These errors are fine, we'll just try again.
            // To view why these errors are ok, read the man page:
            // https://www.openssl.org/docs/manmaster/man3/SSL_read.html
            if ((ssl_err != SSL_ERROR_WANT_READ) && (ssl_err != SSL_ERROR_WANT_WRITE))
            {
                // Errors other than SSL_ERROR_WANT_READ and SSL_ERROR_WANT_WRITE
                // should result in closing the connection and it's associated
                // SSL resources
                SSL_shutdown(ssl_socket);
                SSL_free(ssl_socket);
                bytes_recv = -1;
                break;
            }
        }
    }
    return bytes_recv;
}
int ssl_write_all(SSL * ssl_socket, char * buffer, int byte_count)
{
    printf("This got called");
    int bytes_sent = 0;
    while (bytes_sent != byte_count)
    {
        // echoes back the buffer to the client.
        int written = SSL_write(ssl_socket, buffer, byte_count);
        if (0 > written)
        {
            int ssl_err = SSL_get_error(ssl_socket, written);
            // these errors are fine, we will just keep looping if they occur
            if ((ssl_err != SSL_ERROR_WANT_READ) && (ssl_err != SSL_ERROR_WANT_WRITE))
            {
                SSL_shutdown(ssl_socket);
                SSL_free(ssl_socket);
                bytes_sent = -1;
                break;
            }
        }
        else
        {
            bytes_sent += written;
        }
    }
    return bytes_sent;
}
int main(int argc, char **argv)
{
    struct pollfd fds[MAX_CLIENTS + 1] = {0};
    int nfds = 1;
    init_my_stuff();
    int current_size = 0;
    SSL * ssl[MAX_CLIENTS + 1] = { NULL };

    /* Ignore broken pipe signals */
    //signal(SIGPIPE, SIG_IGN);
    SSL_load_error_strings();
    SSL_CTX * ctx = create_context();
    configure_context(ctx);

    int server_fd = create_socket(SERVER_PORT);
    //set_non_blocking(server_fd);

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
        //printf("new  poll");
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
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        ssl[nfds] = new_ssl;
                        nfds++;
                    }                            
                    else
                    {
                        printf("There has been an error");
                    }
                } 
                else 
                {
                    // Handle data from client
                    char buffer[BUFFER_SIZE] = {0};
                    // normally a loop to handle partial recv would go here, but since we don't know 
                    // how much data to expect, we're ommitting it for now. A program with some
                    // protocol specific header size could instead read the header_size instead
                    // of the BUFFER_SIZE and then make decisions to read more once deserializing
                    // the header.
                    socket_info * my_socket_info = malloc(sizeof(socket_info));

                    my_socket_info->file_descriptor = fds[i];
                    my_socket_info->ssl_socket = ssl[i];
                    my_socket_info->index = i;
                    my_socket_info->num_file_desc = nfds;
                    my_socket_info->fds = fds;
                    printf("here");

                    int recieve = push_sockets(my_socket_info);
                    fds[i] = fds[--nfds];
                    if(recieve == -1)
                    {
                        printf("shoudl not be here\n");
                        close(fds[i].fd);

                    }
                    /*
                    if (0 >= bytes_recv)
                    {
                        printf("client leaving\n");
                        close(fds[i].fd);
                        // This line moves the valid last index in the polling fds
                        // and then moves it to take the place of the fd we just cleaned
                        // up above. It also decrements nfds so that we can still effeciently
                        // add new entries and poll through the valid indices
                        //fds[i] = fds[--nfds];

                        exit(1);
                    }
                    else
                    {
                        // Note: a proper logging system would be better here.
                        printf(" I got good data\n");
                    }
                    */
                }
            }
        }
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}