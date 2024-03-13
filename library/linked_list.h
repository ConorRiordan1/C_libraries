/// @file linked_list.h
/// @brief Defines linked list structs and functins
///@author conor riordan

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/// @brief node in a linked list
typedef struct node
{
    char         username[64];
    char         password[64];
    char         reservations[64][64];
    int          reservation_times[64];
    int          is_admin;
    struct node *next;
} node;

/// @brief LL overhead
typedef struct
{
    struct node *head;
    struct node *tail;
    int          nodecount;
} linkedlist;

// function prototypes
void  initlist(linkedlist *);
node *insertattail (linkedlist * p_listptr, char *username, char *password, int username_len, int password_len);
void  destroylist(linkedlist *lstptr);
void  printlistdetail(linkedlist *);
void  printlist(linkedlist *);
node *
find (linkedlist *p_lstptr, char *username);
node *
find_with_password (linkedlist *p_lstptr,
                    char       *username,
                    char       *password);
char *deletefirst(linkedlist *);
char *deletelast(linkedlist *);
char *deletetarget(linkedlist *, char *username);
#endif //
