/** @file linked_list.c
 *
 * @brief defines functions for linked list operations
 *
 *@par
 * Author: Conor Riordan
 */

#include "linked_list.h"

/// @brief to initialize the linkedlist object
/// @param lstPtr parameter: listp is the pointer to linkedlist object
/// @warning linkedlist object pointed by listptr must be existing.
/// @return Return Value: Nothing
void
initlist (linkedlist *p_lstptr)
{
    p_lstptr->head      = NULL;
    p_lstptr->tail      = NULL;
    p_lstptr->nodecount = 0;
}


/// @brief inserts an element to end of linked list
/// @param p_listptr linked list
/// @param username username
/// @param password password
/// @param username_len username len
/// @param password_len password len
/// @return new node on success, NULL on fail
node *
insertattail (linkedlist * p_listptr, char *username, char *password, int username_len, int password_len)
{
    node * p_node = (node *)malloc(sizeof(node));

    if (p_node == NULL)
    {
        printf("Unable to allocate new node\n");
        return NULL;
    }

    memset(p_node, '\0', sizeof(*p_node));

    strncpy(p_node->username, username, username_len);
    strncpy(p_node->password, password, password_len);
    p_node->next = NULL;


    if (p_listptr->nodecount == 0)
    {
        // this is the case when the list is empty
        p_listptr->head = p_node;
        p_listptr->tail = p_node;
    }
    else
    {
        // this is the case when the list is not empty
        p_listptr->tail->next = p_node;
        p_listptr->tail       = p_node;
    }

    p_listptr->nodecount++;

    return p_node;
}

/// @brief prints a linked list from head to tail w/ pointer information
/// @param lstPtr linked list to be printed
/// @warning linked list has to be initialized before calling this function
void
printlistdetail (linkedlist * p_lstptr)
{
    if (p_lstptr->nodecount == 0)
    {
        printf("Linked list is empty\n");
        return;
    }

    printf("Printing linked list in details\n");
    printf("HEAD: %p\n", p_lstptr->head);

    node *current = p_lstptr->head;
    int   counter = 1;

    while (current != NULL)
    {
        printf("%d. (%p)[%s and %s|%p]\n",
               counter,
               current,
               current->username,
               current->password,
               current->next);
        current = current->next;
        counter++;
    }

    printf("TAIL: %p\n", p_lstptr->tail);
}


/// @brief Function that finds a node in linked list, confirming both username
/// and password match
/// @param p_lstptr pointer to linked list
/// @param username username to search
/// @param password password to search
/// @return NULL if node not found, or pointer to node with matching usernamer +
/// password
/// @warning linked list has to be initialized before calling this function
node *
find_with_password (linkedlist *p_lstptr,
                    char       *username,
                    char       *password)
{

    if (p_lstptr == NULL || p_lstptr->head == NULL)
    {
        goto END; // Handling null pointer
    }

    node *current = p_lstptr->head;


    while (current != NULL)
    {
    
        if ((strlen(current->username) == strlen(username))
            && (strcmp(current->username, username) == 0)
            && (strlen(current->password) == strlen(password))
            && (strcmp(current->password, password) == 0))
        {
            return current; // node found
        }

        current = current->next;
    }

END:
    return NULL; // node not found
}

/// @brief function that searches linked list, finding node with matching
/// username
/// @param lstPtr pointer to linked list
/// @param username username to find
/// @param prvPtr pointer to NULL
/// @return NULL or pointer to found node
/// @warning linked list has to be initialized before calling this function
node *
find (linkedlist *p_lstptr, char *username)
{
    if (p_lstptr == NULL || p_lstptr->head == NULL)
    {
        goto END; // Handling null pointer
    }

    node *current = p_lstptr->head;

    while (current != NULL)
    {
        if ((strlen(current->username) == strlen(username))
            && (strcmp(current->username, username) == 0))
        {
            return current; // node found
        }

        current = current->next;
    }

END:
    return NULL; // node not found
}


static node *
delete_find (linkedlist *p_lstptr,
                    char       *username,
                    node      **pp_prvptr)
{

    if (p_lstptr == NULL || p_lstptr->head == NULL)
    {
        goto END; // Handling null pointer
    }

    node *current = p_lstptr->head;
    *pp_prvptr       = NULL;

    while (current != NULL)
    {
    
        if ((strlen(current->username) == strlen(username))
            && (strcmp(current->username, username) == 0))
        {
            return current; // node found
        }

        *pp_prvptr = current;
        current = current->next;
    }

END:
    return NULL; // node not found
} 

/// @brief function that removes a node from linked list
/// @param lstPtr Pointer to linked list
/// @param username username to remove
/// @return NULL upon failure, username of removed user on success
/// @warning linked list has to be initialized before calling this function
char *
deletetarget (linkedlist *p_lstptr, char *username)
{
    node *current = NULL;
    node * p_prev = NULL;
    current       = delete_find(p_lstptr, username, &p_prev);

    if (current == NULL)
    {
        return NULL;
    }

    char *data = current->username;

    if (current == p_lstptr->head)
    {
        // if the target is the first node
        return deletefirst(p_lstptr);
    }
    else if (current == p_lstptr->tail)
    {
        return deletelast(p_lstptr);
    }
    else
    {

        p_prev->next = current->next;
        free(current);
        p_lstptr->nodecount--;
        return data;
    }
}

/// @brief deletes first element in a linked list
/// @param lstPtr pointer to linked list
/// @return username of elemement removed
/// @warning linked list has to be initialized before calling this function
char *
deletefirst (linkedlist *p_lstptr)
{
    if (p_lstptr->nodecount == 0)
    {
        return NULL;
    }

    node *first = p_lstptr->head;
    char *data  = first->username;

    if (p_lstptr->nodecount == 1)
    {
        // there is only one node
        p_lstptr->head = NULL;
        p_lstptr->tail = NULL;
    }
    else
    {
        // there are many nodes and we need to delete the first
        p_lstptr->head = first->next;
    }
    free(first);
    p_lstptr->nodecount--;
    return data;
}

/// @brief deletes last element in a linked list
/// @param lstPtr pointer to linked list
/// @return username of elemement removed
/// @warning linked list has to be initialized before calling this function
char *
deletelast (linkedlist *p_lstptr)
{
    // if linked list is empty
    if (p_lstptr->nodecount == 0)
    {
        return NULL; // returning absurd value to caller
    }

    node *current = p_lstptr->head;
    node *last    = p_lstptr->tail;
    char *data    = last->username;

    if (p_lstptr->nodecount == 1)
    {
        // if there is only one node, head && tail = NULL
        p_lstptr->head = NULL;
        p_lstptr->tail = NULL;
    }
    else
    {
        // reach just before tail
        while (current->next != p_lstptr->tail)
        {
            current = current->next;
        }

        p_lstptr->tail       = current;
        p_lstptr->tail->next = NULL;
    }
    free(last);
    p_lstptr->nodecount--;
    return data;
}

/// @brief destroys a linked list
/// @param lstPtr pointer to linked list
/// @warning linked list has to be initialized before calling this function
void
destroylist (linkedlist *p_lstptr)
{
    node *current = p_lstptr->head;

    while (current != NULL)
    {
        node *temp = current;
        current    = current->next;
        free(temp);
    }

    // reset Linked list
    p_lstptr->head      = NULL;
    p_lstptr->tail      = NULL;
    p_lstptr->nodecount = 0;
}

/*** end of file***/
