#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "hash.h"

#define MAX_NUM 1000
#define MIN_NUM 1

HashTable *ht1; // space allocated inside library
pthread_mutex_t mutex_for_int_count = PTHREAD_MUTEX_INITIALIZER;

void* count_numbers_thread(void* ptr)
{
    FILE *file;
    //open file
    file = fopen(ptr, "r");
    if(file == NULL)
    {
        printf("\nFailed while opening file.");
        return NULL;
    }
    char line[10];
    int line_size = 10;
    int key_int;
    void *int_count;
    //count the numbers
    while(fgets(line, line_size, file) != NULL)
    {
        key_int = atoi(line);
        //lock
        pthread_mutex_lock(&mutex_for_int_count);
        //int will exists for first time
        if(hash_get(ht1, key_int, &int_count) == -1)
        {
            int_count = malloc(sizeof(int));
            *(int*)int_count = 1;
            hash_insert(ht1, key_int, int_count);
        }
        else
        {
            *(int*)int_count = *(int*)int_count + 1;
            hash_update(ht1, key_int, int_count);
        }
        //unlock
        pthread_mutex_unlock(&mutex_for_int_count);
    }
    pthread_exit(NULL);

}

 int main( int argc, char**argv)
 {
     if (argc <= 2)
     {
         printf("integer-count program usage: as integer-count integer txt.files");
         return -1;
     }
     ht1 = hash_init(MAX_NUM,100);
     int num_of_files = atoi(argv[1]);
     pthread_t threads[num_of_files];
     int i;
     for(i = 0; i < num_of_files; i++)
     {
        pthread_create(&threads[i], NULL, count_numbers_thread, argv[i + 2]);
     }
     for(i = 0; i < num_of_files; i++)
     {
        pthread_join(threads[i], NULL);
     }
     void *int_count;
     FILE *file_to_write;
     file_to_write = fopen(argv[argc - 1], "w");
     for(i = 1; i <= MAX_NUM; i++)
     {
        if(hash_get(ht1, i, &int_count) != -1)
        {
            fprintf(file_to_write, "%d", i);
            fprintf(file_to_write, "%s", ": ");
            fprintf(file_to_write, "%d\n", *(int*)int_count);
        }
     }
     fclose(file_to_write);
     hash_destroy(ht1);
     return 0;

 }
