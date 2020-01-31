
#include <stdio.h>
#include <pthread.h>
#include "hash.h"
#include <stdlib.h>

// Max 100 mutex needed
pthread_mutex_t mutexs[100];
// Mutex for destroy
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

HashTable *hash_init (int N, int K)
{
    //Check for constraints
    if(N >= MIN_N && N <= MAX_N && N % K == 0 && K >= 1 && K <= 100)
    {
        //Create HashTable and allocate space for it
        HashTable *hp;
        hp = (HashTable *) malloc(sizeof(HashTable));
        hp->N = N;
        hp->K = K;
        //Allocate space for buckets
        hp->buckets = (Node**)calloc(N, sizeof(Node*));
        int i;
        //Make all buckets empty

        for(i = 0; i < N; i++)
        {
            hp->buckets[i] = NULL;
        }

        //initilize needed number of mutex
        for(i = 0; i < K; i++)
        {
            pthread_mutex_init(&mutexs[i], NULL);
        }
        return hp;
    }
    printf("\nFailed to create table");
    return (NULL);
}

// key mod(N) returns -1 if key is not valid
int hash_function(int k, int N)
{
    //Check if the key value is valid or not
    if(k < 1)
        return -1;
    return k % N;
}

int hash_insert (HashTable *hp, int k, void *v)
{
    int hash_value = hash_function(k, hp->N);
    //Check if key is valid or not
    if(hash_value == -1)
    {
        printf("\nInvalid Key, Key value must be bigger than 0 - INSERT\n");
        return -1;
    }
    //Found which mutex will be used
    int mutex_value = hash_value / (hp->N / hp->K);
    //---------Lock-------------
    pthread_mutex_lock(&mutexs[mutex_value]);

    //Bucket is empty
    if(hp->buckets[hash_value] == NULL)
    {
        // Allocate
        hp->buckets[hash_value] = (Node*) malloc(sizeof(Node));
        hp->buckets[hash_value]->key = k;
        //new solution
        hp->buckets[hash_value]->value = malloc(sizeof(int));
        *((int*)hp->buckets[hash_value]->value) = *(int*)v;
        hp->buckets[hash_value]->next = NULL;
        //printf("Bucket was empty; First element is inserted\n");
        //-------------Unlock-------------------
        pthread_mutex_unlock(&mutexs[mutex_value]);
        return 0;
    }

    // return -1 if key already exists and on head.
    if(hp->buckets[hash_value]->key == k)
    {
        printf("\nKey already exists.\n");
        //------------------Unlock-------------
        pthread_mutex_unlock(&mutexs[mutex_value]);
        return -1;
    }

    //Create temp_node to hashed bucket's head
    Node *temp_node = hp->buckets[hash_value];

    // return -1 if key already exists.
    while(temp_node->next != NULL)
    {
        if(temp_node->key == k)
        {
            printf("\nKey already exists.\n");
            //------------------Unlock-------------
            pthread_mutex_unlock(&mutexs[mutex_value]);
            return -1;
        }
        temp_node = temp_node->next;
    }

    //Key is not exists and temp_node->next == NULL
    temp_node->next = (Node*) malloc(sizeof(Node));
    temp_node = temp_node->next;
    temp_node->key = k;
    //new solution
    temp_node->value = malloc(sizeof(int));
    *((int*)temp_node->value) = *(int*)v;
    temp_node->value = v;
    temp_node->next = NULL;

    printf ("hash_insert called\n");
    //------------------Unlock-------------
    pthread_mutex_unlock(&mutexs[mutex_value]);
    return (0);
}

int hash_delete (HashTable *hp, int k)
{
    int hash_value = hash_function(k, hp->N);
    //Check if key is valid or not
    if(hash_value == -1)
    {
        printf("\nInvalid Key, Key value must be bigger than 0 - IN DELETE\n");
        return -1;
    }
    //Found which mutex will be used
    int mutex_value = hash_value / (hp->N / hp->K);
    //---------Lock-------------
    pthread_mutex_lock(&mutexs[mutex_value]);

    Node *temp_node = hp->buckets[hash_value];

    // Check if bucket is empty
    if ( temp_node == NULL)
    {
        //printf("Head of the bucket which key could be located == NULL -> key cannot be exist");
        pthread_mutex_unlock(&mutexs[mutex_value]);
        return -1;
    }
    else if (temp_node != NULL)
    {
        //head will be deleted
        if(temp_node->key ==k)
        {
            if(temp_node->next != NULL)
            {
                hp->buckets[hash_value] = temp_node->next;
                //for memory leak
                free(temp_node->value);
                free(temp_node);
                //printf("Head is the deleted node so, head switched to the head->next");
                pthread_mutex_unlock(&mutexs[mutex_value]);
                return 0;
            }
            else
            {
                hp->buckets[hash_value] = NULL;
                //for memory leak
                free(temp_node->value);
                free(temp_node);
                //printf("Only head node is appeared in the bucket! Head deleted - Bucket is empty now\n");
                pthread_mutex_unlock(&mutexs[mutex_value]);
                return 0;
            }
        }
        else
        {
            while (temp_node->next != NULL)
            {
                if( temp_node->next->key ==k)
                {
                    Node* toBeDeleted = temp_node->next;
                    temp_node->next = toBeDeleted->next;
                    //for memory leak
                    free(toBeDeleted->value);
                    free(toBeDeleted);
                    pthread_mutex_unlock(&mutexs[mutex_value]);
                    return 0;
                }
                temp_node = temp_node->next;
            }
        }
    }

    //printf ("hash_delete called\n");
    pthread_mutex_unlock(&mutexs[mutex_value]);
    return -1;
}

int hash_update (HashTable *hp, int k, void *v)
{
    int hash_value = hash_function(k, hp->N);
    //Check if key is valid or not
    if(hash_value == -1)
    {
        printf("\nInvalid Key, Key value must be bigger than 0");
        return -1;
    }

    int mutex_value = hash_value / (hp->N / hp->K);
    //---------Lock-------------
    pthread_mutex_lock(&mutexs[mutex_value]);

    Node *temp_node = hp->buckets[hash_value];

    while(temp_node != NULL)
    {
        if(temp_node->key == k)
        {
            //new solution
            *((int*)temp_node->value) = *(int*)v;
            //temp_node->value = v;
            pthread_mutex_unlock(&mutexs[mutex_value]);
            return 0;
        }
        temp_node = temp_node->next;
    }


    //printf ("hash_update called\n");
    pthread_mutex_unlock(&mutexs[mutex_value]);
    return -1;
}

int hash_get (HashTable *hp, int k, void **vp)
{
    int hash_value = hash_function(k, hp->N);
    //Check if key is valid or not
    if(hash_value == -1)
    {
        printf("\nInvalid Key, Key value must be bigger than 0");
        return -1;
    }
    //Found which mutex will be used
    int mutex_value = hash_value / (hp->N / hp->K);
    //---------Lock-------------
    pthread_mutex_lock(&mutexs[mutex_value]);

    Node *temp_node = hp->buckets[hash_value];

    while(temp_node != NULL)
    {
        if(temp_node->key == k)
        {
            *vp = temp_node->value;
            pthread_mutex_unlock(&mutexs[mutex_value]);
            return 0;
        }
        temp_node = temp_node->next;
    }

    pthread_mutex_unlock(&mutexs[mutex_value]);
    //printf ("hash_get called\n");
    return -1;
}

int hash_destroy (HashTable *hp)
{
    pthread_mutex_lock(&mutex);
    Node *temp_node, *toBeDeleted;
    int i;
    for(i = 0; i < hp->N; i++)
    {
        temp_node = hp->buckets[i];
        while (temp_node != NULL)
        {
            toBeDeleted = temp_node;
            temp_node = toBeDeleted->next;
            //for memory leak
            free(toBeDeleted->value);
            free(toBeDeleted);
        }
    }
    if(hp->buckets != NULL)
    {
        free(hp->buckets);
    }
    free(hp);
    //printf ("hash_destroy called\n");
    pthread_mutex_unlock(&mutex);
    return 0;
}

