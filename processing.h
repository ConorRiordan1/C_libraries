/** @file processing.h
 *
 * @brief header file for processing.c, also defines basic structures
 *
 * Author: Conor Riordan
 */
#ifndef PROCESSING_H
#define PROCESSING_H
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <string.h>
#include "linked_list.h"
#include <openssl/sha.h>
#include <pthread.h>
#include <ctype.h>


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


/// @brief recieved login command format
/// Used for login, register, admin-add, change pass and route
typedef struct login_command_t
{
    char     opcode;
    char     padding[3];
    uint32_t session_id;
    uint16_t username_len;
    uint16_t password_len;
    char     username[64];
    char password[64];
    linkedlist * p_accounts;
    linkedlist * p_maps;
    client_t * client;
} login_command_t;

/// @brief recieved login command format
/// used for link_add and link_del
typedef struct linkadd_command_t
{
    char     opcode;
    char     padding[3];
    uint32_t session_id;
    uint16_t username_len;
    uint16_t password_len;
    char     username[64];
    char password[64];
    uint16_t weight;
    linkedlist * p_accounts;
    linkedlist * p_maps;
    client_t * client;
} linkadd_command_t;



/// @brief recieved user_del
/// used for user_del, list, search location add, locationdel
typedef struct user_del_command_t
{
    char opcode;
    char padding[3];
    uint32_t session_id;
    uint16_t username_len;
    char padding2[2];
    char username[64];
    linkedlist * p_accounts;
    linkedlist * p_maps;
    client_t * client;
}user_del_command_t;


int
process_data (client_t   *my_client,
              char       *buffer,
              size_t         valread,
              linkedlist * p_maplist,
              linkedlist * p_accountlist
              );

int is_ascii(const char *str);
void send_error(client_t * client);
int is_ascii_no_punct(const char *str);

void init_print_mutex(void);
void helgrind_printf(const char *format, const char *username);
#endif