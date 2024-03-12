/** @file proccessing.c
 *
 * @brief sends buffers to threads
 * NOTE: when reading buffers, offsets are used. This are not magic numbers and
 * just the specified offset
 *
 *
 * Author: Conor Riordan
 */

#include "processing.h"

/// @brief sorts login_command_t structs to their proper functions based on
/// opcode!
/// @param command login_command_t
/// @warning use onnly as shown, invalid command will produce undefined output

pthread_mutex_t printf_mutex;
static void sort_login_command(login_command_t *command) {
  if (LOGIN_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_login_command, command);
  } else if (REGISTER_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_register_commmand, command);
  } else if (ADMIN_ADD_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_admin_add_command, command);
  } else if (USER_PASS_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_change_password_command, command);
  } else if (ROUTE_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_route_command, command);
  } else {
    // should not be here but just in case
    free(command);
  }
}

/// @brief sorts user_del_t structs to their proper functions based on opcode!
/// @param command user_del_command_t
/// @warning use onnly as shown, invalid command will produce undefined output
static void sort_user_del_command(user_del_command_t *command) {
  if (USER_DEL_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_user_del_command, command);
  } else if (LIST_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_list_command, command);
  } else if (SEARCH_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_search_command, command);
  } else if (LOCATION_ADD_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_location_add_command, command);
  } else if (LOCATION_DEL_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_location_del_command, command);
  } else {
    free(command);
  }
}

/// @brief sorts linkadd_command_t structs to their proper functions based on
/// opcode!
/// @param command linkadd_command_t
/// @warning use onnly as shown, invalid command will produce undefined output
static void sort_link_add_command(linkadd_command_t *command) {
  if (LINK_ADD_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_link_add_command, command);
  } else if (LINK_DEL_OPCODE == command->opcode) {
    tpool_add_work(p_threadpool, process_link_del_command, command);
  } else {
    free(command);
  }
}

static void validate_type1_command(client_t *my_client, char *buffer,
                                   size_t valread, linkedlist *p_maplist,
                                   linkedlist *p_accountlist) {
  login_command_t *command = calloc(1, sizeof(login_command_t));
  command->p_accounts = p_accountlist;
  command->client = my_client;
  command->p_maps = p_maplist;
  command->opcode = buffer[0];
  command->session_id = ntohl(*(uint32_t *)(buffer + 4));
  command->username_len = ntohs(*(uint16_t *)(buffer + 8));
  command->password_len = ntohs(*(uint16_t *)(buffer + 10));

  if (MAX_NAME_SIZE < command->username_len) // ensure name size valid
  {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  if (MAX_NAME_SIZE < command->password_len) // ensure password size valid
  {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  memcpy(command->username, buffer + 12, command->username_len);
  memcpy(command->password, buffer + 12 + command->username_len,
         command->password_len);
  // we checked lengths to avoid buffer overflow

  // is valid chars
  if (is_ascii(command->username) == 0) {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  // is valid chars
  if (is_ascii(command->password) == 0) {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  // check on strings
  if (strlen(command->username) != command->username_len) {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  // check on strings
  if (strlen(command->password) != command->password_len) {
    send_error(command->client);
    free(command);
    goto EXIT1;
  }

  sort_login_command(command);

EXIT1:
  return 1;
}

static void validate_type2_command(client_t *my_client, char *buffer,
                                   size_t valread, linkedlist *p_maplist,
                                   linkedlist *p_accountlist) {
  user_del_command_t *command = calloc(1, sizeof(user_del_command_t));

  command->opcode = buffer[0];
  command->p_accounts = p_accountlist;
  command->client = my_client;
  command->p_maps = p_maplist;
  command->session_id = ntohl(*(uint32_t *)(buffer + 4));
  command->username_len = ntohs(*(uint16_t *)(buffer + 8));

  if (MAX_NAME_SIZE < command->username_len) {
    send_error(command->client);
    free(command);
    goto EXIT2;
  }
  memcpy(command->username, buffer + 12, command->username_len);

  if (is_ascii(command->username) == 0) {
    send_error(command->client);
    free(command);
    goto EXIT2;
  }

  // check on strings
  if (strlen(command->username) != command->username_len) {
    send_error(command->client);
    free(command);
    goto EXIT2;
  }

  sort_user_del_command(command);

EXIT2:
  return 1;
}

static void validate_type3_command(client_t *my_client, char *buffer,
                                   size_t valread, linkedlist *p_maplist,
                                   linkedlist *p_accountlist) {
  linkadd_command_t *command = calloc(1, sizeof(login_command_t));

  command->p_accounts = p_accountlist;
  command->client = my_client;
  command->p_maps = p_maplist;
  command->opcode = buffer[0];
  command->session_id = ntohl(*(uint32_t *)(buffer + 4));
  command->username_len = ntohs(*(uint16_t *)(buffer + 8));
  command->password_len = ntohs(*(uint16_t *)(buffer + 10));

  if (MAX_NAME_SIZE < command->username_len) // ensure name size valid
  {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  if (MAX_NAME_SIZE < command->password_len) // ensure password size valid
  {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  command->weight = ntohs(*(uint16_t *)(buffer + 12 + command->username_len +
                                        command->password_len));

  memcpy(command->username, buffer + 12, command->username_len);
  memcpy(command->password, buffer + 12 + command->username_len,
         command->password_len);

  // is valid chars
  if (is_ascii(command->username) == 0) {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  // is valid chars
  if (is_ascii(command->password) == 0) {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  // check on strings
  if (strlen(command->username) != command->username_len) {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  // check on strings
  if (strlen(command->password) != command->password_len) {
    send_error(command->client);
    free(command);
    goto EXIT3;
  }

  sort_link_add_command(command);

EXIT3:
  return 1;
}

/// @brief function that gets executed my main thread after data recieved to
/// create threadpool work
/// @param my_client client that sent data
/// @param buffer data client sent
/// @param valread length of data
/// @param p_maplist pointer to map list
/// @param p_accountlist pointer to account list
/// @return 1
int process_data(client_t *my_client, char *buffer, size_t valread,
                 linkedlist *p_maplist, linkedlist *p_accountlist) {
  (void)valread;

  // for OPCODES that require a login_command_t struct
  if (LOGIN_OPCODE == buffer[0] || REGISTER_OPCODE == buffer[0] ||
      ADMIN_ADD_OPCODE == buffer[0] || USER_PASS_OPCODE == buffer[0] ||
      ROUTE_OPCODE == buffer[0])
  {
    validate_type1_command(my_client, buffer, valread, p_maplist,
                           p_accountlist);
  }
  else if (USER_DEL_OPCODE == buffer[0] || LIST_OPCODE == buffer[0] ||
             SEARCH_OPCODE == buffer[0] || LOCATION_ADD_OPCODE == buffer[0] ||
             LOCATION_DEL_OPCODE == buffer[0])
  {
    validate_type2_command(my_client, buffer, valread, p_maplist,
                           p_accountlist);
  }
  else if (LINK_ADD_OPCODE == buffer[0] || LINK_DEL_OPCODE == buffer[0])
  {

    validate_type3_command(my_client, buffer, valread, p_maplist,
                           p_accountlist);
  }
  else
  {
    /// bar c
  }

  return 1;
}

int is_ascii(const char *str) {
  while (*str) {
    if (!isalnum((unsigned char)*str) && !ispunct((unsigned char)*str) &&
        *str != ' ') {
      return 0;
    }
    str++;
  }
  return 1;
}

int is_ascii_no_punct(const char *str) {
  while (*str) {
    if (!isalnum((unsigned char)*str) && *str != ' ') {
      return 0;
    }
    str++;
  }
  return 1;
}

void send_error(client_t *client) {
  char response = UNKNOWN_ERROR;
  pthread_mutex_lock(&client->mutex);
  send(client->socket_fd, &response, sizeof(response), 0);
  pthread_mutex_unlock(&client->mutex);
  return;
}

void helgrind_printf(const char *format, const char *username) {

  pthread_mutex_lock(&printf_mutex);

  if (NULL == username) {
    printf("%s", format);
  } else {
    printf(format, username);
  }
  pthread_mutex_unlock(&printf_mutex);
}

void init_print_mutex(void) { pthread_mutex_init(&printf_mutex, NULL); }

/* END OF FILE*/