#include <stdio.h>
#include "gol.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int size, steps;
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
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

    for (int i = 0; i < steps; i++)
    {
        args_t args_list[n_threads];
        pthread_t threads[n_threads];

        int step = a_size / max_threads;
        int remainder = a_size % max_threads;
        int prev_end = 0;

        for (int t = 0; t < n_threads; t++)
        {
            args_list[t] = args;

            args_list[t].start = prev_end;
            // Executa uma espécie de pidgeonhole sort para que as operações sejam
            // divididas da maneira mais igualitária possível entre as threads
            if (remainder > 0) {
                args_list[t].end = args_list[t].start + step + 1;
                remainder--;
            } else {
                args_list[t].end = args_list[t].start + step;
            }
            prev_end = args_list[t].end;

            args_list[t].board = prev;
            args_list[t].newboard = next;
            args_list[t].size = size;

            pthread_create(&threads[t], NULL, *play, (void *)&args_list[t])
        }

        stats_t stats_step;

        for (int t = 0; t < n_threads; t++)
        {
            pthread_join(&threads[t], &stats_step)
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

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
