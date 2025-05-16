#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

typedef struct
{
    const int *base;
    int *array;
    size_t length;
    ssize_t *output;
} thread_search_t;
int thread_search_back(thread_search_t *args)
{

}

int thread_search(int *array, size_t length, size_t split, ssize_t **output)
{
    if (array == NULL || length == 0 || split == 0 || split > length || output == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    *output = malloc(sizeof(ssize_t) * split);
    pthread_t *threads = malloc(sizeof(pthread_t) * split);
    if (*output == NULL)
        return -1;

    memset(*output, -1, sizeof(ssize_t) * split);

    for (size_t i = 0; i < split; ++i)
    {
        thread_search_t args = {array, array + i * (length / split), length / split, output[i]};
        if (i == split - 1)
            args.length = length / split + length % split;

        if (errno = pthread_create(threads + i, NULL, &thread_search_back, &args))
        {
            free(*output);
            free(threads);
            *output = NULL;
            return -1;
        }
    }
    for (size_t i = 0; i < split; ++i)
    {
        if (errno = pthread_join(threads[i], NULL))
            fprintf(stderr, "Error while waiting thread %i:\t%s", strerror(errno));
    }
    return 0;
}

int main()
{

    return 0;
}