/** @file main.c
 *
 * @brief entry point into server. This manages SIGINT, and startup flags
 *
 *@par
 * Author: Conor Riordan
 */



#include <features.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "network_utils.h"
volatile sig_atomic_t handler_value = 0;

/// @brief Signhandler
/// @param num SIGINT
void sighandler(int num)
{
    write(STDOUT_FILENO, "Ending file.\n", 13);
    handler_value = num;
}



void endless()
{

    while (handler_value == 0)
    {
        server();
        handler_value = 1;
    };

    if (handler_value != 0)
    {
        printf(" I am where i belong\n");
        finish_it();
        return;
    }
}

/// @brief entry point into server
/// @param argc number of arguments
/// @param argv argument vector
/// @return 1
int main()
{

    struct sigaction sa = {.sa_handler = sighandler};

    sigaction(SIGINT, &sa, NULL);

    endless();


    return 1;
}

///*END OF FILE*///
