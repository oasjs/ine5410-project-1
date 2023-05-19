#include <stdio.h>

/*
 * The Game of Life
 *
 * RULES:
 *  1. A cell is born, if it has exactly three neighbours.
 *  2. A cell dies of loneliness, if it has less than two neighbours.
 *  3. A cell dies of overcrowding, if it has more than three neighbours.
 *  4. A cell survives to the next generation, if it does not die of lonelines or overcrowding.
 *
 * In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
 * The game plays a number of steps (given by the input), printing to the screen each time.
 * A 'x' printed means on, space means off.
 *
 */

typedef unsigned char cell_t;

typedef struct {
    unsigned int borns;
    unsigned int overcrowding;
    unsigned int loneliness;
    unsigned int survivals;
} stats_t;

typedef struct
{
    cell_t** board;
    cell_t** newboard;
    int size;
    int firstLine;
    int n_lines;
    stats_t* stats;
} args_t;

// Recebe as barreiras incializadas e o número de iterações
void initialize_game(pthread_barrier_t* wait, pthread_barrier_t* start, int steps);

/* Allocate a GoL board of size = size^2 */
cell_t ** allocate_board(int size);

/* Deallocate a GoL board of size = size^2 */
void free_board(cell_t ** board, int size);

/* Return the number of on cells adjacent to the i,j cell */
int adjacent_to(cell_t ** board, int size, int i, int j);

/* Compute the next generation (newboard) based on the current generation (board) and returns its statistics */
void* play(void* arg);

/* Print the GoL board */
void print_board(cell_t ** board, int size);

/* Print the GoL statistics */
void print_stats(stats_t stats);

/* Read a GoL board from a file */
void read_file(FILE * f, cell_t ** board, int size);