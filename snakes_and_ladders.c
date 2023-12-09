#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY (-1)
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define NUM_OF_ARGS_FROM_INPUT 2
#define WRONG_NUM_OF_ARGS_MSG "Usage: Program should receive 2 arguments"
#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define SEED_VAL_ARG 1
#define DECIMAL 10
#define NUM_OF_ROUTES_ARG 2
#define ROUTE_PRINT_PATTERN "Random Walk %d:"
#define LINE_DOWN "\n"
#define FIRST_CELL_PRINTING_FORMAT_F " [%d] -> "
#define CELL_PRINTING_FORMAT_F "[%d]"
#define CELL_PRINTING_FORMAT_NF "[%d] -> "
#define LADD_TO_PRINTING_FORMAT "[%d]-ladder to %d -> "
#define SNK_TO_PRINTING_FORMAT "[%d]-snake to %d -> "
#define NO_SNK_OR_LADD_IND (-1)

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell
{
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in
    // case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error (char *error_msg, MarkovChain **database)
{
  printf ("%s", error_msg);
  if (database != NULL)
  {
    free_database (database);
  }
  return EXIT_FAILURE;
}

static int create_board (Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc (sizeof (Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++)
      {
        free (cells[j]);
      }
      handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell *cells[BOARD_SIZE];
  if (create_board (cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database (markov_chain, cells[i]);
  }
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database (markov_chain, cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
      to_node = get_node_from_database (markov_chain,
                                        cells[index_to])
          ->data;
      add_node_to_frequencies_list (from_node,
                                    to_node, markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell *) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database (markov_chain,
                                          cells[index_to])
            ->data;
        add_node_to_frequencies_list (from_node, to_node,
                                      markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free (cells[i]);
  }
  return EXIT_SUCCESS;
}

static void routes_printer (MarkovChain *markov_chain, int num_of_routes)
{
  for (int i = 1; i <= num_of_routes; i++)
  {
    printf (ROUTE_PRINT_PATTERN, i);
    generate_tweet (markov_chain, markov_chain->database->first->data,
                    MAX_GENERATION_LENGTH);
    printf ("\n");
  }
}

/**
 * This function prints the details of a single cell in the game.
 * It takes a void pointer as a parameter.
*/
static void lets_print_a_single_cell (void *cell)
{
  Cell *casted = (Cell *) cell;
  int cell_num = casted->number;
  int ladd = casted->ladder_to;
  int snk = casted->snake_to;

  if (cell_num == 1)
  {
    printf (FIRST_CELL_PRINTING_FORMAT_F, cell_num);
    return;
  }
  if (cell_num == BOARD_SIZE)
  {
    printf (CELL_PRINTING_FORMAT_F, cell_num);
    return;
  }
  if (ladd != NO_SNK_OR_LADD_IND)
  {
    printf (LADD_TO_PRINTING_FORMAT, cell_num, ladd);
  }
  if (snk != NO_SNK_OR_LADD_IND)
  {
    printf (SNK_TO_PRINTING_FORMAT, cell_num, snk);
  }
  if (snk == NO_SNK_OR_LADD_IND && ladd == NO_SNK_OR_LADD_IND)
  {
    printf (CELL_PRINTING_FORMAT_NF, cell_num);
  }
}

/**
 * This function compares two cells and returns the difference
 * in their cell numbers.
 * It takes two void pointers as parameters, which are
 * casted to Cell structs.
*/
static int comperor (void *first_cell, void *second_cell)
{
  Cell *casted_first = ((Cell *) first_cell);
  Cell *casted_second = ((Cell *) second_cell);
  int comparison = casted_first->number - casted_second->number;
  return comparison;
}

/**
 * This function checks whether a cell is the last cell in the
 * game and returns a boolean.
 * It takes a void pointer as a parameter, which is casted to a Cell struct.
*/
static bool finish_checker (void *cell)
{
  Cell *casted_cell = (Cell *) cell;
  bool finished = (casted_cell->number == BOARD_SIZE);
  return finished;
}

/**
* prints ALLOCATION_ERROR_MASSAGE upon failure in allocating new memory.
*/
static void memory_allocation_failed_printer ()
{
  printf (ALLOCATION_ERROR_MASSAGE);
}

/**
 * This function allocates memory for a string and copies the given data into
 * the allocated memory. It takes a void pointer as a parameter.
*/
static void *copy_allocator (void *data)
{
  if (!data)
  {
    return NULL;
  }
  Cell *allocation = malloc (sizeof (Cell));
  if (allocation == NULL)
  {
    memory_allocation_failed_printer ();
    return NULL;
  }
  *(allocation) = (Cell) {((Cell *) data)->number, ((Cell *) data)->ladder_to,
                          ((Cell *) data)->snake_to};
  return allocation;
}

/**
* Creates a new markov-chain.
*/
static MarkovChain *markov_chain_creator ()
{
  MarkovChain *mc = calloc (1, sizeof (MarkovChain));
  if (mc == NULL)
  {
    memory_allocation_failed_printer ();
    return NULL;
  }
  return mc;
}

/**
* Creates a new database.
*/
static LinkedList *database_creator ()
{
  LinkedList *db = calloc (1, sizeof (LinkedList));
  if (db == NULL)
  {
    memory_allocation_failed_printer ();
    return NULL;
  }
  return db;
}

/**
 * This function sets up the MarkovChain for the game by modifying its fields
 * It takes two parameters, a MarkovChain and a LinkedList.
*/
static void game_setter (MarkovChain *markov_chain, LinkedList *database)
{
  markov_chain->database = database;
  database->first = NULL;
  database->last = NULL;
  markov_chain->print_func = lets_print_a_single_cell;
  markov_chain->comp_func = comperor;
  markov_chain->free_data = free;
  markov_chain->copy_func = copy_allocator;
  markov_chain->is_last = finish_checker;
}

/**
 * This function checks the number of command-line arguments passed to the
 * program
 * and returns a boolean indicating whether they are correct.
 * It takes two parameters,
 * an integer representing the number of arguments passed and an
 * array of strings
 * containing the arguments.
*/
static int input_checker (int argc)
{
  bool args_are_good = true;
  if (argc != NUM_OF_ARGS_FROM_INPUT + INPUT_FACTOR)
  {
    printf (WRONG_NUM_OF_ARGS_MSG);
    args_are_good = false;
  }
  return args_are_good;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  if (input_checker (argc) == false)
  {
    return EXIT_FAILURE;
  }
  unsigned int seed = strtol (argv[SEED_VAL_ARG], NULL,
                              DECIMAL);
  srand (seed);

  int num_of_routes = strtol (argv[NUM_OF_ROUTES_ARG],
                              NULL, DECIMAL);
  MarkovChain *mar_chain = markov_chain_creator ();
  LinkedList *data = database_creator ();
  if (!mar_chain || !data)
  {
    free (mar_chain);
    free (data);
    return EXIT_FAILURE;
  }
  game_setter (mar_chain, data);
  if (fill_database (mar_chain) == EXIT_FAILURE)
  {
    free_database (&mar_chain);
    return EXIT_FAILURE;
  }
  routes_printer (mar_chain, num_of_routes);
  free_database (&mar_chain);
  return EXIT_SUCCESS;
}
