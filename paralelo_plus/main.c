#include <stdio.h>
#include "gol.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro>!\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    fscanf(f, "%d %d", &size, &steps);

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    // Define o número de threads
    int n_threads = atoi(argv[2]);
    if (n_threads > size)
        n_threads = size;

    // Inicializa as variáveis para a distribuição de pombal
    int step = size / n_threads;
    int remainder = size % n_threads;
    int prev_end = 0;

    /* sem_t* semaphores = malloc(sizeof(sem_t) * n_threads); */
    sem_t* semaphores[n_threads];
    pthread_mutex_t* counterMutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(counterMutex, NULL);

    // Inicializa os valores fixos e os insere em args.
    args_t args[n_threads];
    pthread_t threads[n_threads];
    for (int i = 0; i < n_threads; i++) 
    {
        args[i].firstLine = prev_end;
        prev_end = args[i].firstLine + step;
        if (remainder > 0) {
            ++prev_end;
            --remainder;
        }
        args[i].n_lines = prev_end - args[i].firstLine;

        args[i].size = size;
        args[i].steps = steps;
        args[i].n_threads = n_threads;

        semaphores[i] = malloc(sizeof(sem_t));
        sem_init(semaphores[i], 0, 1);
        args[i].semaphores = semaphores;
        args[i].semIndex = i;
        args[i].counterMutex = counterMutex;
    
        args[i].board = prev;
        args[i].newboard = next;
        // Uso calloc para inicializar os stats em 0
        args[i].stats = (stats_t*)calloc(1, sizeof(stats_t));
        pthread_create(&threads[i], NULL, *play, (void*)&args[i]);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
        stats_total.borns           += args[i].stats->borns;
        stats_total.survivals       += args[i].stats->survivals;
        stats_total.loneliness      += args[i].stats->loneliness;
        stats_total.overcrowding    += args[i].stats->overcrowding;
        sem_destroy(semaphores[i]);
        free(semaphores[i]);
        free(args[i].stats);
    }
    pthread_mutex_destroy(counterMutex);
    free(counterMutex);
    tmp = next;
    next = prev;
    prev = tmp;

// #ifdef DEBUG
//         printf("Step %d ----------\n", i + 1);
//         print_board(next, size);
//         print_stats(stats_step);
// #endif

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
