/** @file main.c
 *
 * @brief main file for server init
 *
 *@par
 * Author: Conor Riordan
 */
#include "network_utils.h"
#include "signal.h"
#include "threadpool.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
volatile sig_atomic_t g_handler_value = 0;
tpool_t *p_threadpool;

/// @brief Signhandler
/// @param num SIGINT
void sighandler(int num) {
  write(STDOUT_FILENO, "Ending file.\n", 13);
  g_handler_value = num;
}

void sigpipe_handler(int signum) {
  (void)signum;
  // Ignore sigpipe
}

void endless(char *menu_path, uint32_t port, char *user_file, int ipv) {
  linkedlist *p_lstptr = NULL;
  linkedlist *p_accountptr = NULL;
  while (g_handler_value == 0) {

    // init_session_ids();
    // p_threadpool = tpool_create(1);
    server(p_lstptr, p_accountptr, port, ipv);
    g_handler_value = 1;
  };

  if (g_handler_value != 0) {
    printf("hi\n");
    // tpool_destroy(p_threadpool);
    // write_map(menu_path, p_lstptr);
    // write_accounts(user_file, p_accountptr);
    // delete_map(p_lstptr, MAP_LIST);
    // delete_map(p_accountptr, ACCOUNT_LIST);
    fflush(stdout);
    cleanup_sockets();
    return;
  }
}

int main(int argc, char *argv[]) {

  int opt;
  char *menu_path = "maps/large_map";
  uint32_t port = 8000;
  char *user_file = "users.csv";
  int ipv = 4;

  while ((opt = getopt(argc, argv, ":m:p:u:6h")) != -1) {
    switch (opt) {
    case 'm':
      menu_path = optarg;
      break;
    case 'p':
      port = 8000;
      break;
    case 'u':
      user_file = optarg;
      break;
    case '6':
      ipv = 6;
      break;
    case 'h':
      printf("USAGE: ./server [option] ...\n options and arguments\n"
             "-m\t menu path\n-p\t port number\n-u\tuser file path"
             "\n-6\t ipv6 enabled\n");
      goto EXIT;
    case '?':
      printf("unknown option%c, see help for help\n", optopt);
      goto EXIT;
    case ':':
      printf("missing args for this: %c\n", optopt);
      goto EXIT;
    }
  }

  struct sigaction sa_sigint = {.sa_handler = sighandler};
  struct sigaction sa_pipe = {.sa_handler = sigpipe_handler};

  sigaction(SIGINT, &sa_sigint, NULL);
  sigaction(SIGPIPE, &sa_pipe, NULL);
  endless(menu_path, port, user_file, ipv);

EXIT:
  return 1;
}

// end of file
