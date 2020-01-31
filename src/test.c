#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "hash.h"

HashTable *ht; // space allocated inside library

int t; //number of threads
int w; //number of keys/operations
int k; //number of locks
int n; //table size

void* hash_table_operations(void* ptr)
{
    int i;
    void *val = malloc(sizeof(int));
    void *v = malloc(sizeof(int));
    void *update = malloc(sizeof(int));
    for (i = 1; i <= w ; i++)
    {
        *(int*)val = i + 5;
        hash_insert (ht, i, val);
        printf("\nhash_insert called: key = ");
        printf("%d", i);
        printf(" value = ");
        printf("%d\n", *(int*)val);

        *(int*)update = i;
        if(hash_update(ht,i,update) != -1)
        {
            printf("\nhash_updated called: key = ");
            printf("%d", i);
            printf(" updated to value = ");
            printf("%d\n", *(int*)update);
        }


        if(hash_get(ht, i, &v) != -1)
        {
            printf("\nhash_get called: key = ");
            printf("%d", i);
            printf(" received value = ");
            printf("%d\n",*(int*)v);
        }
    }
    pthread_exit(NULL);
}
int main(int argc, char **argv)
{
    int i;

    if (argc < 5){
        printf("Usage: ./test T W K N \n");
    }
    else{
        //initialize the variables with given values to test
        t = atoi(argv[1]);
        w = atoi(argv[2]);
        k = atoi(argv[3]);
        n = atoi(argv[4]);
        //create hash table with given values
        ht = hash_init (n, k);
        if(ht == NULL)
        {
            return 0;
        }
        pthread_t threads[t]; //create threads with given thread number
        for (i =0; i < t; i++)
        {
            int thread_num = i;
            pthread_create(&threads[i], NULL, hash_table_operations, &thread_num);
        }

        for (i =0; i < t; i++){
            pthread_join(threads[i], NULL);
        }

        hash_destroy (ht);
        return 0;
    }
}

