#include <stdio.h>
#include "gol.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
    stats_t stats_step;
    int n_threads = 12; 

    if (argc != 2)
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
    print_stats(stats_step);
#endif

    // Aloca um array de tamanho n_threads para os argumentos
    args_t* argumentos = (args_t *)malloc(sizeof(args_t) * n_threads);

    // Loop da iteração
    for (int i = 0; i < steps; i++) {

        pthread_t threads[n_threads];

        int step = size / n_threads;
        int remainder = size % n_threads;
        int prev_end = 0;


        for (int t = 0; t < n_threads; t++) {

            argumentos[t].start = prev_end;

            /* Executa uma espécie de pidgeonhole sort para que as operações sejam
               divididas da maneira mais igualitária possível entre as threads */
            if (remainder > 0) {
                argumentos[t].end = argumentos[t].start + step + 1;
                remainder--;
            } else {
                argumentos[t].end = argumentos[t].start + step;
            }
            prev_end = argumentos[t].end;

            argumentos[t].board = prev;
            argumentos[t].newboard = next;
            argumentos[t].size = size;

            pthread_create(&threads[t], NULL, *play, (void *)&argumentos[t]);
        }

        for (int t = 0; t < n_threads; t++) {
            pthread_join(threads[t], (void *)&stats_step);
            stats_total.borns += stats_step.borns;
            stats_total.survivals += stats_step.survivals;
            stats_total.loneliness += stats_step.loneliness;
            stats_total.overcrowding += stats_step.overcrowding;
        }

#ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(next, size);
        print_stats(stats_step);
#endif
        tmp = next;
        next = prev;
        prev = tmp;
    }

free (argumentos);

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
