// AVL tree implementation in C

#include <stdio.h>
#include <stdlib.h>

// Create node
struct node
{
    int           key;
    struct node * left;
    struct node * right;
    int           height;
};

int max(int a, int b);

// Calculate height
int
height(struct node * p_node)
{
    if (p_node == NULL)
        return 0;
    return p_node->height;
}

int
max(int a, int b)
{
    return (a > b) ? a : b;
}

// Create a node
struct node *
newnode(int key)
{
    struct node * p_node = (struct node *)malloc(sizeof(struct node));
    p_node->key          = key;
    p_node->left         = NULL;
    p_node->right        = NULL;
    p_node->height       = 1;
    return (p_node);
}

// Right rotate
struct node *
rightRotate(struct node * p_ydx)
{
    struct node * p_xdx  = p_ydx->left;
    struct node * p_tdx2 = p_xdx->right;

    p_xdx->right = p_ydx;
    p_ydx->left  = p_tdx2;

    p_ydx->height = max(height(p_ydx->left), height(p_ydx->right)) + 1;
    p_xdx->height = max(height(p_xdx->left), height(p_xdx->right)) + 1;

    return p_xdx;
}

// Left rotate
struct node *
leftRotate(struct node * p_xdx)
{
    struct node * p_ydx  = p_xdx->right;
    struct node * p_t2dx = p_ydx->left;

    p_ydx->left  = p_xdx;
    p_xdx->right = p_t2dx;

    p_xdx->height = max(height(p_xdx->left), height(p_xdx->right)) + 1;
    p_ydx->height = max(height(p_ydx->left), height(p_ydx->right)) + 1;

    return p_ydx;
}

// Get the balance factor
int
getBalance(struct node * p_node)
{
    if (p_node == NULL)
        {
            return 0;
        }

    return height(p_node->left) - height(p_node->right);
}

// Insert node
struct node *
insertnode(struct node * p_node, int key)
{
    // Find the correct position to insertnode the node and insertnode it

    if (p_node == NULL)
    {
        return (newnode(key));
    }

    if (key < p_node->key)
    {
        p_node->left = insertnode(p_node->left, key);
    }
    else if (key > p_node->key)
    {
        p_node->right = insertnode(p_node->right, key);
    }
    else
    {
        return p_node;
    }

    // Update the balance factor of each node and
    // Balance the tree
    p_node->height = 1 + max(height(p_node->left), height(p_node->right));

    int balance = getBalance(p_node);

    if (balance > 1 && key < p_node->left->key)
    {
        return rightRotate(p_node);
    }

    if (balance < -1 && key > p_node->right->key)
    {
        return leftRotate(p_node);
    }

    if (balance > 1 && key > p_node->left->key)
    {
        p_node->left = leftRotate(p_node->left);
        return rightRotate(p_node);
    }

    if (balance < -1 && key < p_node->right->key)
    {
        p_node->right = rightRotate(p_node->right);
        return leftRotate(p_node);
    }

    return p_node;
}

struct node *
minValuenode(struct node * p_node)
{
    struct node * p_current = p_node;

    while (p_current->left != NULL)
    {
        p_current = p_current->left;
    }

    return p_current;
}

// Delete a nodes
struct node *
deletenode(struct node * p_root, int key)
{
    // Find the node and delete it

    if (p_root == NULL)
    {
        return p_root;
    }

    if (key < p_root->key)
    {
        p_root->left = deletenode(p_root->left, key);
    }
    else if (key > p_root->key)
    {
        p_root->right = deletenode(p_root->right, key);
    }
    else
    {

        if ((NULL == p_root->left) || (NULL == p_root->right))
        {
            struct node * p_temp = p_root->left ? p_root->left : p_root->right;

            if (NULL == p_temp)
            {
                p_temp = p_root;
                p_root = NULL;
            }
            else
                *p_root = *p_temp;
            free(p_temp);
        }
        else
        {
            struct node * p_temp = minValuenode(p_root->right);

            p_root->key = p_temp->key;

            p_root->right = deletenode(p_root->right, p_temp->key);
        }
    }

    if (NULL == p_root)
    {
        return p_root;
    }

    // Update the balance factor of each node and
    // balance the tree
    p_root->height = 1 + max(height(p_root->left), height(p_root->right));

    int balance = getBalance(p_root);

    if (balance > 1 && getBalance(p_root->left) >= 0)
    {
        return rightRotate(p_root);
    }

    if (balance > 1 && getBalance(p_root->left) < 0)
    {
        p_root->left = leftRotate(p_root->left);
        return rightRotate(p_root);
    }

    if (balance < -1 && getBalance(p_root->right) <= 0)
        return leftRotate(p_root);

    if (balance < -1 && getBalance(p_root->right) > 0)
    {
        p_root->right = rightRotate(p_root->right);
        return leftRotate(p_root);
    }

    return p_root;
}

// Print the tree
void
printPreOrder(struct node * p_root)
{
    if (p_root != NULL)
    {
        printf("%d ", p_root->key);
        printf("Balance factor is: %d for node %d\n",getBalance(p_root),p_root->key);
        printPreOrder(p_root->left);
        printPreOrder(p_root->right);
    }
}

void
deleteTree(struct node * root)
{
    if (root == NULL)
        return;

    /* first delete both subtrees */
    deleteTree(root->left);
    deleteTree(root->right);

    /* then delete the node */
    printf("\n Deleting node: %d", root->key);
    free(root);
}

int
main()
{
    struct node * root = NULL;

    root = insertnode(root, 2);
    root = insertnode(root, 1);
    root = insertnode(root, 7);
    root = insertnode(root, 4);
    root = insertnode(root, 5);
    root = insertnode(root, 3);
    root = insertnode(root, 8);

    printPreOrder(root);

    root = deletenode(root, 3);

    printf("\nAfter deletion: ");
    printPreOrder(root);
    deleteTree(root);

    return 0;
}