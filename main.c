#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Linked List node
struct node {

	// key is string
	char* key;

	// value is also string
	char* value;
	struct node* next;
};



// like constructor
void setNode(struct node* node, char* key, char* value)
{
	node->key = key;
	node->value = value;
	node->next = NULL;
	return;
};

struct hashMap {

	// Current number of elements in hashMap
	// and capacity of hashMap
	int numOfElements, capacity;

	// hold base address array of linked list
	struct node** arr;
};
void merge_hashtable(struct hashMap * hashtable_old, struct hashMap * hashtable_new);
void
destroyList(struct node * Node)

{
    struct node * current = Node;
    while (current != NULL)
    {
        struct node * temp = current;
        current     = current->next;
        free(temp);
    }

    // Reset the linked list attributes

}

void delete_list(struct hashMap * hashtable)
{
    for(int idx = 0; idx <= hashtable->capacity;idx++)
    {
            destroyList(hashtable->arr[idx]);
            //free(hashtable->arr);

    }
    free(hashtable->arr);
    free(hashtable);
}

// like constructor
void initializeHashMap(struct hashMap* mp,int capacity)
{

	// Default capacity in this case
	mp->capacity = capacity;
	mp->numOfElements = 0;

	// array of size = 1
	mp->arr = (struct node**)malloc(sizeof(struct node*)
									* mp->capacity);
	return;
}

int hashFunction(struct hashMap* mp, char* key)
{
	int bucketIndex;
	int sum = 0, factor = 31;
	for (int i = 0; i < strlen(key); i++) {

		// sum = sum + (ascii value of
		// char * (primeNumber ^ x))...
		// where x = 1, 2, 3....n
		sum = ((sum % mp->capacity)
			+ (((int)key[i]) * factor) % mp->capacity)
			% mp->capacity;

		// factor = factor * prime
		// number....(prime
		// number) ^ x
		factor = ((factor % __INT16_MAX__)
				* (31 % __INT16_MAX__))
				% __INT16_MAX__;
	}
	bucketIndex = sum;

	return bucketIndex;
}

struct hashMap * insert(struct hashMap* mp, char* key, char* value)
{

	// Getting bucket index for the given
	// key - value pair
	int bucketIndex = hashFunction(mp, key);
	struct node* newNode = (struct node*)malloc(

		// Creating a new node
		sizeof(struct node));

	// Setting value of node
	setNode(newNode, key, value);

	// Bucket index is empty....no collision
	if (mp->arr[bucketIndex] == NULL) {
		mp->arr[bucketIndex] = newNode;
	}

	// Collision
	else {

		// Adding newNode at the head of
		// linked list which is present
		// at bucket index....insertion at
		// head in linked list
		newNode->next = mp->arr[bucketIndex];
		mp->arr[bucketIndex] = newNode;
	}
    mp->numOfElements++;
    float fload = (float)mp->numOfElements/(float)mp->capacity;
    if(fload>=0.75)
    {
    struct hashMap * mp_new = (struct hashMap*)malloc(sizeof(struct hashMap));
	initializeHashMap(mp_new,mp->capacity*2.0);
    merge_hashtable(mp,mp_new);
    printf("hi");
	delete_list(mp);
	mp = mp_new;
    //memcpy(mp,mp_new,sizeof(struct hashMap));
    }
	return mp;
}

void merge_hashtable(struct hashMap * hashtable_old, struct hashMap * hashtable_new)
{
    for(int idx = 0; idx <= hashtable_old->capacity;idx++)
    {
            struct node *head = hashtable_old->arr[idx];
            struct node *temp = head;
            while(NULL != head)
            {
                insert(hashtable_new,head->key,head->value);
				temp = head;
                head = head->next;
                temp = head;
            }


    }
	//free(hashtable_old);
    //delete_list(hashtable_old);
}

void delete (struct hashMap* mp, char* key)
{

	// Getting bucket index for the
	// given key
	int bucketIndex = hashFunction(mp, key);

	struct node* prevNode = NULL;

	// Points to the head of
	// linked list present at
	// bucket index
	struct node* currNode = mp->arr[bucketIndex];

	while (currNode != NULL) {

		// Key is matched at delete this
		// node from linked list
		if (strcmp(key, currNode->key) == 0) {

			// Head node
			// deletion
			if (currNode == mp->arr[bucketIndex]) {
				mp->arr[bucketIndex] = currNode->next;
			}

			// Last node or middle node
			else {
				prevNode->next = currNode->next;
			}
			free(currNode);
			break;
		}
		prevNode = currNode;
		currNode = currNode->next;
	}
	return;
}

char* search(struct hashMap* mp, char* key)
{
    char* errorMssg = (char*)malloc(sizeof(char) * 25);
	// Getting the bucket index
	// for the given key
	int bucketIndex = hashFunction(mp, key);

	// Head of the linked list
	// present at bucket index
	struct node* bucketHead = mp->arr[bucketIndex];
	while (bucketHead != NULL) {

		// Key is found in the hashMap
		if (bucketHead->key == key) {
            strcpy(errorMssg,bucketHead->value);
			return errorMssg;
		}
		bucketHead = bucketHead->next;
	}

	// If no key found in the hashMap
	// equal to the given key
	strcpy(errorMssg,"Opps, no data found");
	return errorMssg;
}

void print_hashtable(struct hashMap * hashtable)
{
    for(int idx = 0; idx <= hashtable->capacity;idx++)
    {
            struct node *head = hashtable->arr[idx];
            while(NULL != head)
            {
                printf("Key: %s, value %s\n",head->key,head->value);
                head = head->next;
            }

    }
}

// Drivers code
int main()
{

	// Initialize the value of mp
	struct hashMap* mp
		= (struct hashMap*)malloc(sizeof(struct hashMap));
	initializeHashMap(mp,2);

	mp = insert(mp, "Yogaholic", "Anjali");
	mp = insert(mp, "pluto14", "Vartika");
	mp = insert(mp, "elite_Programmer", "Manish");
	mp = insert(mp, "decentBoy", "Mayank");
	mp = insert(mp, "Yo2gaholic", "Anjali");
	mp = insert(mp, "pludto14", "Vartika");
	mp = insert(mp, "elitse_Programmer", "Manish");
	mp = insert(mp, "decendftBoy", "Mayank");
	char * message = search(mp,"pludto14");
	printf(" I found%s",message);
	free(message);
	delete (mp, "decentBoy");
    print_hashtable(mp);
    delete_list(mp);

	return 0;
}
