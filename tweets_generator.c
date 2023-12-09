//
// Created by adish on 4/22/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include "markov_chain.h"
#include <string.h>
#define SEED_VAL_ARG 1
#define NUM_OF_TWEETS_TO_TWEET_ARG 2
#define PATH_TO_TEXT_CORPUS_ARG 3
#define NUM_OF_WORDS_TO_READ_FROM_TEXT 4
#define MIN_NUM_OF_ARGS_FROM_INPUT 3
#define MAX_NUM_OF_ARGS_FROM_INPUT 4
#define INPUT_FACTOR 1
#define INVALID_PATH_ERROR_MSG "Error: File is invalid or inaccessible"
#define WRONG_NUM_OF_ARGS_MSG "Usage: Program should receive either 3 or 4 "\
"arguments"
#define DECIMAL 10
#define MAX_LEN_OF_SENTENCE 1001
#define MAX_LEN_OF_WORD 101
#define DELIM " \r\n"
#define LARGEST_INT ((1u<<31) - 1u)
#define MIN_NUM_OF_WORDS 2
#define MAX_NUM_OF_WORDS_IN_SINGLE_TWEET 20
#define SENTENCE_FINISHER '.'
#define TWEET_STARTER_FORMAT "Tweet"

/**
* prints ALLOCATION_ERROR_MASSAGE upon failure in allocating new memory.
*/
static void memory_allocation_failed_printer ()
{
  printf (ALLOCATION_ERROR_MASSAGE);
}

/**
* The function db_fill_failure is a static function that is used to handle
 * the failure of filling the Markov chain database.
*/
static int db_fill_failure (FILE *fp, MarkovChain *markov_chain)
{
  fclose ((FILE *) fp);
  free_database (&markov_chain);
  free (markov_chain);
  return false;
}

/**
* Reads file line-by-line and adds to db if word isn't there yet.
*/
static int file_reader (FILE *fp, int words_to_read,
                        MarkovChain *markov_chain)
{
  // Reading file
  char sentence[MAX_LEN_OF_SENTENCE];
  while (words_to_read > 0
         && fgets (sentence, MAX_LEN_OF_SENTENCE, fp) != NULL)
  {
    char *single_word = strtok (sentence, DELIM);
    while ((words_to_read > 0) && single_word != NULL)
    {
      if (add_to_database (markov_chain, single_word) == NULL)
      {
        printf (ALLOCATION_ERROR_MASSAGE);
        return db_fill_failure (fp, markov_chain);
      }
      single_word = strtok (NULL, DELIM);
      words_to_read--;
    }
  }
  return true;
}

/**
* Initializes an array to contain nothing but zeros.
*/
static void init_arr (char *arr)
{
  for (int i = 0; i < MAX_LEN_OF_SENTENCE; i++)
  {
    arr[i] = 0;
  }
}

/**
* Responsible for updating a word's frequency list.
* Goes line by line and word by word, adding words to frequencies list.
*/
static int freq_updater (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  char full_line[MAX_LEN_OF_SENTENCE];
  while (words_to_read > 0 && fgets (full_line,
                                     MAX_LEN_OF_SENTENCE, fp) != NULL)
  {
    char *word1 = strtok (full_line, DELIM);
    char *word2 = strtok (NULL, DELIM);
    int inner_counter = MIN_NUM_OF_WORDS;
    while (words_to_read - inner_counter >= 0 && word1 != NULL
           && word2 != NULL)
    {
      if (markov_chain->is_last (word1) != true)
      {
        MarkovNode *node1 = (get_node_from_database (markov_chain,
                                                     word1))
            ->data;
        MarkovNode *node2 = (get_node_from_database (markov_chain,
                                                     word2))
            ->data;
        if (add_node_to_frequencies_list (node1, node2,
                                          markov_chain)
            == false)
        {
          return db_fill_failure (fp, markov_chain);
        }
      }
      inner_counter++;
      word1 = word2;
      word2 = strtok (NULL, DELIM);
    }
    words_to_read -= (inner_counter - 1);
    init_arr (full_line);
  }
  return true;
}

/**
* Responsible for filling the database in words, returns true upon success,
* false otherwise.
*/
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  if (file_reader (fp, words_to_read, markov_chain) == true)
  {
    fseek (fp, 0, SEEK_SET);
    if (freq_updater (fp, words_to_read, markov_chain) == true)
    {
      return true;
    }
  }
  return false;
}

/**
* Responsible for comparing 2 tweets, returns 0 upon success
*/
static int tweet_comperor (void *tweet1, void *tweet2)
{
  const char *casted_tweet1 = (const char *) tweet1;
  const char *casted_tweet2 = (const char *) tweet2;
  return strcmp (casted_tweet1, casted_tweet2);
}

/**
 * A function that  gets a pointer of generic data type and returns a newly
 * allocated copy of it returns a generic pointer.
*/
static void *copy_alloc_tweet (void *data)
{
  char *allocation = malloc (strlen ((char *) data) + 1);
  if (allocation == NULL)
  {
    memory_allocation_failed_printer ();
    return NULL;
  }
  strcpy (allocation, data);
  return allocation;
}

/**
 * A function that returns True if a word ends with a period
*/
static bool finish_tweet_checker (void *single_word)
{
  char *casted_cell = (char *) single_word;
  size_t word_length = strlen (casted_cell);
  if (word_length == 0)
  {
    return false;
  }
  char *final_let = &(casted_cell[word_length - 1]);
  bool finished = (*final_let == SENTENCE_FINISHER);
  return finished;
}

/**
 * A function that prints tweets
*/
static void adjusted_tweet_printer (void *tweet)
{
  char *casted_tweet = (char *) tweet;
  printf ("%s", casted_tweet);
  if (finish_tweet_checker (casted_tweet) == false)
  {
    printf (" ");
    return;
  }
}

/**
* Initializes markov-node's initial properties.
*/
static void markov_init_properties_setter (MarkovChain *mc, LinkedList *db)
{
  mc->database = db;
  db->first = NULL;
  db->last = NULL;
  mc->print_func = adjusted_tweet_printer;
  mc->comp_func = tweet_comperor;
  mc->free_data = free;
  mc->copy_func = copy_alloc_tweet;
  mc->is_last = finish_tweet_checker;
}

/**
* Checks if number of args from input is valid.
*/
static int valid_num_of_args (int argc)
{
  if (argc != (MIN_NUM_OF_ARGS_FROM_INPUT + INPUT_FACTOR) &&
      argc != (MAX_NUM_OF_ARGS_FROM_INPUT + INPUT_FACTOR))
  {
    printf (WRONG_NUM_OF_ARGS_MSG);
    return false;
  }
  return true;
}

/**
* Returns true upon success in opening path, false otherwise.
*/
static int valid_file_to_open (char *path)
{
  FILE *file_to_open = fopen (path, "r");
  if (file_to_open == NULL)
  {
    printf (INVALID_PATH_ERROR_MSG);
    return false;
  }
  fclose (file_to_open);
  return true;
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
* Main's function, runs generate tweet function till no more tweets are
* needed to be made.
*/
static void run_tweets (int num_of_tweets_to_make, MarkovChain *mar_chain)
{
  int counter = 1;
  while (num_of_tweets_to_make > 0)
  {
    printf (TWEET_STARTER_FORMAT" %d: ", counter);
    counter++;
    generate_tweet (mar_chain, NULL,
                    MAX_NUM_OF_WORDS_IN_SINGLE_TWEET);
    num_of_tweets_to_make--;
    printf ("\n");
  }
}

/**
* Main function of the program.
*/
int main (int argc, char *argv[])
{
  if (valid_num_of_args (argc) == false)
  {
    return EXIT_FAILURE;
  }
  // First argument, seed
  unsigned int seed = strtol (argv[SEED_VAL_ARG], NULL,
                              DECIMAL);
  srand (seed);
  // Second argument, Tweets to make
  int num_of_tweets_to_make = strtol (argv[NUM_OF_TWEETS_TO_TWEET_ARG],
                                      NULL, DECIMAL);
  // Third argument, Text Corpus
  char *path = argv[PATH_TO_TEXT_CORPUS_ARG];
  if (valid_file_to_open (path) == false)
  {
    return EXIT_FAILURE;
  }
  // Fourth argument, Number of words to read
  int num_of_words_to_read = LARGEST_INT;
  if (argc == MAX_NUM_OF_ARGS_FROM_INPUT + INPUT_FACTOR)
  {
    num_of_words_to_read = strtol (argv[NUM_OF_WORDS_TO_READ_FROM_TEXT],
                                   NULL, DECIMAL);
  }
  // Allocating memory for markov chain and database
  MarkovChain *mar_chain = markov_chain_creator ();
  LinkedList *data = database_creator ();
  if (!mar_chain || !data)
  {
    free (mar_chain);
    free (data);
    return EXIT_FAILURE;
  }
  markov_init_properties_setter (mar_chain, data);
  FILE *file_to_open = fopen (path, "r");
  if (fill_database (file_to_open, num_of_words_to_read,
                     mar_chain) == false)
  {
    free_database (&mar_chain);
    return EXIT_FAILURE;
  }
  run_tweets (num_of_tweets_to_make, mar_chain);
  free_database (&mar_chain);
  fclose (file_to_open);
  return EXIT_SUCCESS;
}
