/** @file network_utils.c
 *
 * @brief defines all operations related to server init and managment
 *
 *@par
 * Author: Conor Riordan
 */
#include "network_utils.h"

struct pollfd fds[CONN_MAX];         // pollfd for pooling
int           server_fd;             // server file descriptor
client_t *    client_sockets;        // customized client_sockets for later use
int           total_connections = 0; // num of people on server

/// @brief creats ipv6 server
/// @param my_port port to listen to
/// @return socket, -1 on failure
static int
create_socket6(int my_port)
{
    helgrind_printf("launching ipv6 enabled server\n", NULL);
    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address)); // Initialize to zero

    server_fd = socket(AF_INET6, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        printf("Socket Failed");
        goto EXIT;
    }

    int option = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))
        == -1)
    {
        printf("Error setting socket options");
        goto EXIT;
    }

    server_address.sin6_family = AF_INET6;
    server_address.sin6_port   = htons(my_port);
    server_address.sin6_addr   = in6addr_any;

    if (bind(server_fd,
             (struct sockaddr *)&server_address,
             sizeof(server_address))
        == -1)
    {
        printf("Failed to bind socket");
        goto EXIT;
    }

    if (listen(server_fd, CONN_MAX) == -1)
    {
        printf("Failed to listen");
        goto EXIT;
    }

    return server_fd;

EXIT:

    if (server_fd != -1)
    {
        close(server_fd);
    }

    return -1;
}

/// @brief creates ipv4 server
/// @param my_port port to listen to
/// @return socket, -1 on error
static int
create_socket4(int my_port)
{
    helgrind_printf("launching ipv4 enabled server\n", NULL);
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if ( 0 > server_fd)
    {
        printf("Socket Failed");
        goto EXIT;
    }

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(my_port);

    /// allowing reuse of socket
    int yes = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
        == -1)
    {
        close(server_fd);
        printf("Error setting socket options");
        goto EXIT;
    }

    /// binding
    if (0 > bind(server_fd, (struct sockaddr *)&address, sizeof(address)))
    {
        close(server_fd);
        printf("Failed to bind socket");
        goto EXIT;
    }

    /// listening
    if (0 > listen(server_fd, CONN_MAX))
    {
        close(server_fd);
        printf("failed to listen");
        goto EXIT;
    }

    return server_fd;

EXIT:
    return -1;
}

/// @brief initializes sockets for use
/// @param socket socket to be initialized
static void
init_socket(client_t * socket)
{
    socket->last_message_time = 0;
    socket->socket_fd         = 0;
    socket->session_id        = 0;
    pthread_mutex_init(&(socket->mutex), NULL);
}

/// @brief resets a socket for use
/// @param socket socket to be reset
static void
reset_socket(client_t * socket)
{

    close(socket->socket_fd);
    socket->last_message_time = 0;
    socket->socket_fd         = 0;
    socket->session_id        = 0;
}

/// @brief checks all active sockets and disconnects them if timeout
/// @param clients list of clients
/// @param num_clients number of clients
static void
check_for_timeouts(client_t * clients, int num_clients)
{
    time_t current_time = time(NULL);

    for (int idx = 1; idx < num_clients; ++idx) // look int 1 or zeo
    {

        if ((clients[idx].socket_fd > 0))
        {
            time_t elapsed_time = current_time - clients[idx].last_message_time;

            if (elapsed_time > TIMEOUT_LIMIT)
            {
                helgrind_printf("client timeout\n", NULL);
				total_connections = total_connections - 1;
                pthread_mutex_lock(&clients[idx].mutex);
                reset_socket(&clients[idx]);
                pthread_mutex_unlock(&clients[idx].mutex);
            }
        }
    }
}

static void
process_existing_client(int          idx,
                        size_t       max_recv_size,
                        linkedlist * p_maplist,
                        linkedlist * p_accountlist)
{
    char * buffer = calloc(1, max_recv_size);

    if (NULL == buffer)
    {
        helgrind_printf("error\n", NULL);
        goto EXIT;
    }

    pthread_mutex_lock(&client_sockets[idx].mutex);

    size_t valread
        = recv(client_sockets[idx].socket_fd, buffer, max_recv_size, 0);

    pthread_mutex_unlock(&client_sockets[idx].mutex);

    // client has left the session
    if (0 == valread || valread > max_recv_size)
    {
        total_connections = total_connections - 1;
        pthread_mutex_lock(&client_sockets[idx].mutex);
        reset_socket(&client_sockets[idx]);
        pthread_mutex_unlock(&client_sockets[idx].mutex);
        free(buffer);
        printf("A client has left or been booted\n");
    }
    // client has sent something
    else
    {
        client_sockets[idx].last_message_time = time(NULL);
		printf("i got data\n");
        process_data(
            &client_sockets[idx], buffer, valread, p_maplist, p_accountlist);
        free(buffer);
    }

EXIT:
    return;
}

static void
handle_new_connection(int new_socket)
{
    /// adding our new client to client_sockets
    for (int idx = 1; idx < CONN_MAX; idx++)
    {
        pthread_mutex_lock(&client_sockets[idx].mutex);

        if (0 == client_sockets[idx].socket_fd)
        {
            client_sockets[idx].socket_fd         = new_socket;
            client_sockets[idx].last_message_time = time(NULL);
            fds[idx].fd                           = new_socket;
            fds[idx].events                       = POLLIN;
            fds[idx].revents                      = 0;
            pthread_mutex_unlock(&client_sockets[idx].mutex);
            break;
        }

        pthread_mutex_unlock(&client_sockets[idx].mutex);
    }
}

/// @brief begins sever and polls for connections
/// @param p_maplist list of planets
/// @param p_accountlist list of users
/// @param port listening port
/// @param ipv ipv4 or 6
/// @return -1 on failure
int
server(linkedlist * p_maplist, linkedlist * p_accountlist, int port, int ipv)
{

    // init of basics, no connections, port is provided, new_socket used to
    // represent client
    int my_port    = port; // port server hosted on
    int new_socket = 0;    // client connection

    // Allows for helgrind to be used when stdout is used in threads
    init_print_mutex();

    // Max size in bytes a client can send
    size_t max_recv_size = 100;

    // custom array of structs used to represent client
    client_sockets = (client_t *)malloc(CONN_MAX * sizeof(client_t));

    if (client_sockets == NULL)
    {
        printf("Memory allocation failed.\n");
        goto EXIT;
    }

    // init client_t structs
    for (int idx = 0; idx < CONN_MAX; idx++)
    {
        init_socket(&client_sockets[idx]);
    }

    // our server
    server_fd = -1;

    if (IPV6 == ipv)
    {
        server_fd = create_socket6(my_port);
    }
    else
    {
        server_fd = create_socket4(my_port);
    }

    if (-1 == server_fd)
    {
        printf("server init failed\n");
        goto EXIT;
    }

    fds[0].fd      = server_fd; // communicated with on new connection
    fds[0].events  = POLLIN;    /// trigger on input
    fds[0].revents = 0;         /// occurances

    // zeroize everything else besides fds[0] because they represent clients
    for (int idx = 1; idx < CONN_MAX; idx++)
    {
        memset(&fds[idx], 0, sizeof(fds[idx]));
    }

    // our main loop
    for (;;)
    {
        int waiting = poll(fds, CONN_MAX, POLL_RETURN_RATE);

        if (-1 == waiting)
        {
            close(server_fd);
            printf("listen error\n");
            goto EXIT;
        }

        // conditional triggered on new connection only + enough space to add
        // new connection
        if ((fds[0].revents & POLLIN) && (CONN_MAX > total_connections))
        {
            struct sockaddr_in addr;
            unsigned int       len = sizeof(addr);

            if (((new_socket = accept(
                      server_fd, (struct sockaddr *)&addr, (socklen_t *)&len))
                 < 0))
            {
                printf("failing to accept\n");
                goto EXIT;
            }
            else
            {
                printf("New connection\n");
				total_connections++;
                printf("Num connections %d\n", total_connections);
                handle_new_connection(new_socket);
            }
        }

        // loop monitors existing connections
        for (int idx = 1; idx < CONN_MAX; idx++)
        {

            extern volatile sig_atomic_t g_handler_value;

            // server not checking for SIGINT after client disconnect, so we
            // double check here.
            if (0 != g_handler_value)
            {
                goto EXIT;
            }

            /// Triggered if an exisitng connection sends something
            if (fds[idx].revents & POLLIN)
            {
                process_existing_client(
                    idx, max_recv_size, p_maplist, p_accountlist);
            }
        }

        // will trigger every POLL_RETURN RATE to boot clients
        check_for_timeouts(client_sockets, CONN_MAX);
    }

EXIT:
    return -1;
}

/// @brief closes all sockets
/// @return 1
int
cleanup_sockets()
{
    for (int idx = 0; idx < CONN_MAX; idx++)
    {

        pthread_mutex_destroy(&(client_sockets[idx].mutex));
        close(client_sockets[idx].socket_fd);
    }
    free(client_sockets);
    client_sockets = NULL;
    close(server_fd);

    return 1;
}

/* END OF FILE*/