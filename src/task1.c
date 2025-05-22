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
    int *output;
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
            int index = args->array - args->base + i;
            if (args->output != NULL)
                *args->output = index;

            printf("index: %i & value = %i\n", index, args->array[i]);
            return index;
        }
    }
    puts("not found");

    return -1;
}

int thread_find_first(int *array, size_t length, size_t depth, int target, int invert)
{
    if (array == NULL || length == 0 || depth == 0 || depth > length)
    {
        errno = EINVAL;
        return -1;
    }

    int *output = malloc(sizeof(int) * depth);
    if (output == NULL)
        return -1;

    pthread_t *threads = malloc(sizeof(pthread_t) * depth);
    if (threads == NULL)
    {
        free(output);
        return -1;
    }

    thread_search_t *args = malloc(sizeof(pthread_t) * depth);
    if (args == NULL)
    {
        free(output);
        free(threads);
        return -1;
    }

    memset(output, -1, sizeof(int) * depth);

    for (size_t i = 0; i < depth; ++i)
    {
        args[i] = (thread_search_t){array, array + i * (length / depth), output + i, length / depth, target, invert};
        if (i == depth - 1)
            args[i].length = length / depth + length % depth;

        if ((errno = pthread_create(threads + i, NULL, &thread_search_back, args + i)) != 0)
        {
            fprintf(stderr, "Error creating a thread %i:\t%s", strerror(errno));
            free(output);
            free(threads);
            free(args);
            output = NULL;
            return -1;
        }
    }
    for (size_t i = 0; i < depth; ++i)
        if ((errno = pthread_join(threads[i], NULL)) != 0)
            fprintf(stderr, "Error while waiting thread %i:\t%s", strerror(errno));

    free(args);
    free(threads);

    size_t depth_half = depth / 2;
    if (depth_half == 0)
        return output[0];

    int index = -1;
    if (depth_half == 1)
    {
        thread_search_t final_args = {output, output, NULL, depth, -1, 1};
        index = thread_search_back(&final_args);
        printf("%i\n", index);
    }
    else
        index = thread_find_first(output, depth, depth_half, -1, 1);

    if (index == -1)
        return -1;
    else
        index = output[index];

    free(output);
    return index;
}

int main()
{
    int array[] = {1, 2, 3, 4, 5, 6, 7, 4, 9, 10, 11, 12, 4, 4, 15};
    size_t length = sizeof(array) / sizeof(int);
    size_t depth = 4;
    size_t target = 4;
    size_t first_index = thread_find_first(array, length, depth, target, 0);

    printf("First element %i at %i\n", target, first_index);

    return 0;
}