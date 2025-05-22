#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

typedef struct
{
    pthread_mutex_t *mutex;
    size_t *last_index;
    size_t *capacity;
    const int *base;
    int *array;
    int **output;
    size_t length;
    int target;
    int invert_search;
} thread_search_t;
static int thread_search_back(thread_search_t *args)
{
    for (size_t i = 0; i < args->length; ++i)
    {
        if (!args->invert_search && (args->target == args->array[i]) ||
            args->invert_search && (args->target != args->array[i]))
        {
            pthread_mutex_lock(args->mutex);

            if (*args->last_index >= *args->capacity)
                *args->output = reallocarray(*args->output, (*args->capacity *= 2), sizeof(int));

            if (*args->output == NULL)
            {
                pthread_mutex_unlock(args->mutex);
                return -1;
            }

            (*args->output)[(*args->last_index)++] = args->array - args->base + i;

            pthread_mutex_unlock(args->mutex);
        }
    }

    return 0;
}

int thread_find_all(
    int *array,
    size_t length,
    size_t depth,
    int target,
    int invert,
    int **output,
    size_t *output_size)
{
    if (array == NULL || length == 0 || depth == 0 || depth > length || output == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    size_t capacity = 1;
    *output = malloc(sizeof(int));
    if (*output == NULL)
        return -1;

    pthread_t *threads = malloc(sizeof(pthread_t) * depth);
    if (threads == NULL)
    {
        free(*output);
        return -1;
    }

    thread_search_t *args = malloc(sizeof(thread_search_t) * depth);
    if (args == NULL)
    {
        free(*output);
        free(threads);
        return -1;
    }

    **output = -1;
    size_t last_index = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    for (size_t i = 0; i < depth; ++i)
    {
        args[i] = (thread_search_t){
            &mutex,
            &last_index,
            &capacity,
            array,
            array + i * (length / depth),
            output,
            length / depth,
            target,
            invert
        };
        if (i == depth - 1)
            args[i].length = length / depth + length % depth;

        if ((errno = pthread_create(threads + i, NULL, &thread_search_back, args + i)) != 0)
        {
            fprintf(stderr, "Error creating a thread %i:\t%s", i, strerror(errno));
            free(*output);
            free(threads);
            free(args);
            *output = NULL;
            return -1;
        }
    }
    for (size_t i = 0; i < depth; ++i)
        if ((errno = pthread_join(threads[i], NULL)) != 0)
            fprintf(stderr, "Error while waiting thread %i:\t%s", strerror(errno));

    *output_size = last_index;
    if (capacity != last_index)
        *output = reallocarray(*output, last_index, sizeof(int));

    free(args);
    free(threads);
    if ((errno = pthread_mutex_destroy(&mutex)) != 0)
        fprintf(stderr, "Error destroying a mutex:\t%s", strerror(errno));

    return 0;
}

int main()
{
    int array[] = {1, 2, 3, 4, 5, 6, 7, 4, 9, 10, 11, 12, 4, 4, 15};
    size_t length = sizeof(array) / sizeof(int);
    size_t depth = 4;
    int target = 4;
    int *output = NULL;
    size_t output_size = 0;
    int status = thread_find_all(
        array,
        length,
        depth,
        target,
        0,
        &output,
        &output_size);

    for (size_t i = 0; i < output_size; ++i)
        printf("%i ", output[i]);
    puts("");

    return 0;
}