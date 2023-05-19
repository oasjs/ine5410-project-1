#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "gol.h"

int main(int argc, char **argv)
{
    cell_t **prev, **next, **tmp;
    FILE *f;
    stats_t stats_total = {0, 0, 0, 0};
    pthread_barrier_t barrier_wait, barrier_start;

    int size, steps;

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

    // Inicializa as barreiras
    pthread_barrier_init(&barrier_wait, NULL, n_threads);
    pthread_barrier_init(&barrier_start, NULL, n_threads);
    initialize_game(&barrier_wait, &barrier_start, steps);

    // Inicializa as variáveis para a distribuição de pombal
    int step = size / n_threads;
    int remainder = size % n_threads;
    int prev_end = 0;

    // Inicializa os valores de firstLine, n_lines e size de cada arg.
    // Isso é feito fora do loop de steps pois esses valores são os mesmos ao
    // longo de todo o jogo.
    args_t args[n_threads];
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
    }

    pthread_t threads[n_threads];

    // Faz a inicialização das threads
    for (int j = 0; j < n_threads; j++) 
    {
        args[j].board = prev;
        args[j].newboard = next;
        // Uso calloc para inicializar os stats em 0
        args[j].stats = (stats_t*)calloc(1, sizeof(stats_t));
        pthread_create(&threads[j], NULL, *play, (void*)&args[j]);
    }

    // Faz a troca de tabuleiros após o término de cada iteração
    for (int i = 0; i < steps; i++) {
        printf("pre");
        fflush(stdout);
        pthread_barrier_wait(&barrier_wait);
        printf("trancou");
        fflush(stdout);
        tmp = next;
        next = prev;
        prev = tmp;

        #ifdef DEBUG
        printf("Step %d ----------\n", i + 1);
        print_board(next, size);
        //print_stats(stats_step);
        #endif
        pthread_barrier_wait(&barrier_start);
        printf("saiu");
        fflush(stdout);
    }   

    // Dá join em todas as threads e acumula em stats_step o resultado
    // de cada thread.
    stats_t stats_step = {0, 0, 0, 0};
    for (int j = 0; j < n_threads; j++) {
        pthread_join(threads[j], NULL);
        stats_step.borns           += args[j].stats->borns;
        stats_step.survivals       += args[j].stats->survivals;
        stats_step.loneliness      += args[j].stats->loneliness;
        stats_step.overcrowding    += args[j].stats->overcrowding;
        free(args[j].stats);
    }   
    stats_total.borns           += stats_step.borns;
    stats_total.survivals       += stats_step.survivals;
    stats_total.loneliness      += stats_step.loneliness;
    stats_total.overcrowding    += stats_step.overcrowding;

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}
