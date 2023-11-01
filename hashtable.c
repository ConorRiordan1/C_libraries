#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Linked List node
typedef struct node
{

    // key is stringdelete
    char * key;

    // value is also string
    char *        value;
    struct node * p_next;
} node;


typedef struct hashmap
{

    // Current number of elements in hashMap
    // and capacity of hashMap
    int numOfElements;
	int capacity;

    // hold base address array of linked list
    node ** pp_arr;
} hashmap;

// like constructor
void
setNode(node * node, char * key, char * value)
{
    node->key   = key;
    node->value = value;
    node->p_next  = NULL;
    return;
};

void merge_hashtable(hashmap * hashtable_old, hashmap * hashtable_new);

void
destroyList(node * p_Node)

{
    node * p_current = p_Node;

    while (NULL != p_current)
    {
        node * temp = p_current;
        p_current     = p_current->p_next;
        free(temp);
    }

    // Reset the linked list attributes
}

void
delete_list(hashmap * p_hashtable)
{
    for (int idx = 0; idx <= p_hashtable->capacity-1; idx++)
    {
        destroyList(p_hashtable->pp_arr[idx]);

    }

    free(p_hashtable->pp_arr);
    free(p_hashtable);
}

// like constructor
void
initializeHashMap(hashmap * p_mp, int capacity)
{


    p_mp->capacity      = capacity;
    p_mp->numOfElements = 0;

    // array of size = 1
    p_mp->pp_arr = (node **)malloc(sizeof(node *) * p_mp->capacity);
	for (int idx = 0; idx<capacity;idx++)
	{
		p_mp->pp_arr[idx] = NULL;
	}
    return;
}

int
hashFunction(hashmap * p_mp, char * key)
{
    int bucketIndex;
    int sum = 0;
	int factor = 31;

    for (int idx = 0; idx < strlen(key); idx++)
    {

        // sum = sum + (ascii value of
        // char * (primeNumber ^ x))...
        // where x = 1, 2, 3....n
        sum = ((sum % p_mp->capacity) + (((int)key[idx]) * factor) % p_mp->capacity)
              % p_mp->capacity;

        // factor = factor * prime
        // number....(prime
        // number) ^ x
        factor
            = ((factor % __INT16_MAX__) * (31 % __INT16_MAX__)) % __INT16_MAX__;
    }
    bucketIndex = sum;

    return bucketIndex;
}

hashmap *
insert(hashmap * p_mp, char * key, char * value)
{

    // Getting bucket index for the given
    // key - value pair
    int    bucketIndex = hashFunction(p_mp, key);
    node * newNode     = (node *)malloc(

        // Creating a new node
        sizeof(node));

    // Setting value of node
    setNode(newNode, key, value);

    // Bucket index is empty....no collision
    if (p_mp->pp_arr[bucketIndex] == NULL)
    {
        p_mp->pp_arr[bucketIndex] = newNode;
    }

    // Collision
    else
    {

        // Adding newNode at the head of
        // linked list which is present
        // at bucket index....insertion at
        // head in linked list
        newNode->p_next        = p_mp->pp_arr[bucketIndex];
        p_mp->pp_arr[bucketIndex] = newNode;
    }
    p_mp->numOfElements++;

    float fload = (float)p_mp->numOfElements / (float)p_mp->capacity;
    if (fload >= 0.75)
    {
        hashmap * p_mp_new = (hashmap *)malloc(sizeof(hashmap));
        initializeHashMap(p_mp_new, p_mp->capacity * 2.0);
        merge_hashtable(p_mp, p_mp_new);
        delete_list(p_mp);
        p_mp = p_mp_new;
    }

    return p_mp;
}

void
merge_hashtable(hashmap * hashtable_old, hashmap * hashtable_new)
{
    for (int idx = 0; idx <= hashtable_old->capacity-1; idx++)
    {
        node * head = hashtable_old->pp_arr[idx];
        node * temp = head;
        while (NULL != head)
        {
            insert(hashtable_new, head->key, head->value);
            //temp = head;
            head = head->p_next;
            temp = head;
        }
    }
}

void delete(hashmap * mp, char * key)
{

    // Getting bucket index for the
    // given key
    int bucketIndex = hashFunction(mp, key);

    node * prevNode = NULL;

    // Points to the head of
    // linked list present at
    // bucket index
    node * currNode = mp->pp_arr[bucketIndex];

    while (currNode != NULL)
    {

        // Key is matched at delete this
        // node from linked list
        if (strcmp(key, currNode->key) == 0)
        {

            // Head node
            // deletion
            if (currNode == mp->pp_arr[bucketIndex])
            {
                mp->pp_arr[bucketIndex] = currNode->p_next;
            }

            // Last node or middle node
            else
            {
                prevNode->p_next = currNode->p_next;
            }
            free(currNode);
            break;
        }
        prevNode = currNode;
        currNode = currNode->p_next;
    }
    return;
}

char *
search(hashmap * mp, char * key)
{
    char * errorMssg = (char *)malloc(sizeof(char) * 25);
    // Getting the bucket index
    // for the given key
    int bucketIndex = hashFunction(mp, key);

    // Head of the linked list
    // present at bucket index
    node * bucketHead = mp->pp_arr[bucketIndex];
    while (bucketHead != NULL)
    {

        // Key is found in the hashMap
        if (bucketHead->key == key)
        {
            strcpy(errorMssg, bucketHead->value);
            return errorMssg;
        }
        bucketHead = bucketHead->p_next;
    }

    // If no key found in the hashMap
    // equal to the given key
    strcpy(errorMssg, "Opps, no data found");
    return errorMssg;
}

void
print_hashtable(hashmap * hashtable)
{
    for (int idx = 0; idx <= hashtable->capacity-1; idx++)
    {
        node * head = hashtable->pp_arr[idx];
        while (NULL != head)
        {
            printf("Key: %s, value %s\n", head->key, head->value);
            head = head->p_next;
        }
    }
}

// Drivers code
int
main()
{

    // Initialize the value of mp
    hashmap * mp = (hashmap *)malloc(sizeof(hashmap));
    initializeHashMap(mp, 2);

    mp             = insert(mp, "Yogaholic", "Anjali");
    mp             = insert(mp, "pluto14", "Vartika");
    mp             = insert(mp, "elite_Programmer", "Manish");
    mp             = insert(mp, "decentBoy", "Mayank");
    mp             = insert(mp, "Yo2gaholic", "Anjali");
    mp             = insert(mp, "pludto14", "Vartika");
    mp             = insert(mp, "elitse_Programmer", "Manish");
    mp             = insert(mp, "decendftBoy", "Mayank");
    char * message = search(mp, "pludto14");
    printf(" I found%s", message);
    free(message);
    delete (mp, "decentBoy");
    print_hashtable(mp);
    delete_list(mp);

    return 0;
}
