//
// Created by adish on 4/21/2023.
//
#include "markov_chain.h"
#include "linked_list.h"
#include <stdio.h>  // For printf(), sscanf()
#include <stdlib.h> // For exit(), malloc()
#include <stdbool.h> // for bool
#define NO_LENGTH 0
#define NEW_FREQ_VAL_INIT 1
#define INIT_LEN_OF_FREQ_LST 1
#define TWEET_INIT_LENGTH 1

/**
* Checks if data_ptr is in database. If so, returns the markov_node
* wrapping it in
* the markov_chain, otherwise return NULL.
* @param markov_chain the chain to look in its database
* @param data_ptr the state to look for
* @return Pointer to the Node wrapping given state, NULL if state not in
* database.
*/
Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *seeker_node = markov_chain->database->first;
  while (seeker_node != NULL)
  {
    if (markov_chain->comp_func (seeker_node->data->data, data_ptr) == 0)
    {
      return seeker_node;
    }
    seeker_node = seeker_node->next;
  }
  return NULL;
}

/**
* Returns true if markov-node's allocation had succeeded, prints allocation
* error massage and returns false otherwise.
*/
bool node_allocation_success (MarkovNode *new_node)
{
  if (new_node == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return false;
  }
  return true;
}

/**
* Creates a new markov-node with all needed properties.
*/
MarkovNode *new_node_creator (MarkovChain *markov_chain,
                              MarkovNode *new_node, void *data_ptr)
{
  void *word_taker = markov_chain->copy_func (data_ptr);
  if (word_taker == NULL)
  {
    return NULL;
  }
  new_node->data = word_taker;
  new_node->frequencies_list = NULL;
  new_node->len_of_frequency_list = NO_LENGTH;
  new_node->finisher = markov_chain->is_last (data_ptr);
  if (add (markov_chain->database, new_node) == 1)
  {
    free (new_node->frequencies_list);
    markov_chain->free_data (word_taker);
    return NULL;
  }
  return new_node;
}

/**
* If data_ptr in markov_chain, return it's node. Otherwise, create new
* node, returns NULL in case of memory allocation failure..
* @param markov_chain the chain to look in its database
* @param data_ptr the state to look for
* @return markov_node wrapping given data_ptr in given chain's database,
* returns NULL in case of memory allocation failure.
*/
Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *req_node = get_node_from_database (markov_chain, data_ptr);
  if (req_node != NULL)
  {
    return req_node;
  }
  MarkovNode *new_mnode = malloc (sizeof (MarkovNode));
  // returns NULL in case of memory allocation failure.
  if (node_allocation_success (new_mnode) == false)
  {
    free (new_mnode);
    return NULL;
  }
  MarkovNode *adjusted_mn = new_node_creator (markov_chain, new_mnode,
                                              data_ptr);
  if (adjusted_mn == NULL)
  {
    free (new_mnode);
    return NULL;
  }
  return markov_chain->database->last;
}

/**
* add_node_to_frequencies_list's helper, responsible for reallocating
* memory if freq list exists, allocates new memory for second node in freq
* list if freq list doesn't exist.
*/
bool frequency_list_memory_allocation (MarkovNode *first_node, MarkovNode
*second_node)
{
  // Allocating memory for second node in freq list if freq list exists.
  if (first_node->len_of_frequency_list != 0)
  {
    MarkovNodeFrequency *new_freq_list = realloc
        (first_node->frequencies_list, sizeof
                                           (MarkovNodeFrequency)
                                       * (first_node->len_of_frequency_list
                                          + 1));
    if (new_freq_list == NULL)
    {
      printf (ALLOCATION_ERROR_MASSAGE);
      return false;
    }
    first_node->frequencies_list = new_freq_list;
    first_node->len_of_frequency_list++;
    first_node->frequencies_list[first_node->len_of_frequency_list -
                                 1].markov_node = second_node;
    first_node->frequencies_list[first_node->len_of_frequency_list -
                                 1].frequency = NEW_FREQ_VAL_INIT;
    return true;
  }
    // Allocating memory for second node in freq list if freq list Doesn't
    // exist.
  else
  {
    first_node->frequencies_list = calloc (1, sizeof
                                                  (MarkovNodeFrequency) + 1);
    if (first_node->frequencies_list == NULL)
    {
      printf (ALLOCATION_ERROR_MASSAGE);
      return false;
    }
    first_node->len_of_frequency_list = INIT_LEN_OF_FREQ_LST;
    first_node->frequencies_list[first_node->len_of_frequency_list -
                                 1].markov_node = second_node;
    first_node->frequencies_list[first_node->len_of_frequency_list -
                                 1].frequency = NEW_FREQ_VAL_INIT;
    return true;
  }
}

/**
* Add the second markov_node to the frequency list of the first markov_node.
* If already in list, update it's occurrence frequency value.
* @param first_node
* @param second_node
* @return success/failure: true if the process was successful, false if in
* case of allocation error.
*/
bool add_node_to_frequencies_list (MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  // Just making sure None of the nodes is NULL, May be unnecessary
  if (!first_node || !second_node)
  {
    return false;
  }
  // Checking if second markov node is already in frequency list of first
  // markov node.
  for (int i = 0; i < first_node->len_of_frequency_list; i++)
  {
    if (markov_chain->comp_func (second_node->data,
                           first_node->frequencies_list[i].markov_node->data)
        == 0)
    {
      first_node->frequencies_list[i].frequency++;
      return true;
    }
  }
  return frequency_list_memory_allocation (first_node, second_node);
}

/**
* Free markov_chain and all of it's content from memory
* @param markov_chain markov_chain to free
*/
void free_database (MarkovChain **ptr_chain)
{
  if (ptr_chain == NULL || *ptr_chain == NULL)
  {
    return;
  }
  LinkedList *db = (*ptr_chain)->database;
  if (db != NULL)
  {
    Node *curr = db->first;
    while (curr != NULL)
    {
      MarkovNode *cur_data = curr->data;
      if (cur_data != NULL)
      {
        if (cur_data->frequencies_list != NULL)
        {
          free (cur_data->frequencies_list);
        }
        if (cur_data->data != NULL)
        {
          (*ptr_chain)->free_data (cur_data->data);
        }
        free (cur_data);
      }
      Node *tmp = curr;
      curr = curr->next;
      free (tmp);
    }
    free (db);
  }
  free (*ptr_chain);
  *ptr_chain = NULL;
}

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number (int max_number)
{
  return rand () % max_number;
}

/**
* Get one random state from the given markov_chain's database.
* @param markov_chain
* @return
*/
MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  Node *curr_node;
  while (true)
  {
    int num_o_nodes = markov_chain->database->size;
    int u_dist_number = get_random_number (num_o_nodes);
    curr_node = markov_chain->database->first;
    for (int i = 0; i < u_dist_number; i++)
    {
      curr_node = curr_node->next;
      if (curr_node == NULL)
      {
        break;
      }
    }
    if (markov_chain->is_last (curr_node->data->data) == false)
    {
      break;
    }
  }
  return curr_node->data;
}

/**
* Checks if next random node is NULL;
*/
int get_next_random_node_nullity_checker (int freq, MarkovNode
*state_struct_ptr)
{
  if (freq == 0 || !(state_struct_ptr->frequencies_list))
  {
    return false;
  }
  return true;
}

/**
* Choose randomly the next state, depend on it's occurrence frequency.
* @param state_struct_ptr MarkovNode to choose from
* @return MarkovNode of the chosen state
*/
MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  int freq = 0;
  int nodes_count = state_struct_ptr->len_of_frequency_list;
  if (!(state_struct_ptr->frequencies_list))
  {
    return NULL;
  }
  int freq_counter = 0;
  while (freq_counter < nodes_count)
  {
    freq += (state_struct_ptr->frequencies_list)[freq_counter].frequency;
    freq_counter++;
  }
  int rand_getter = get_random_number (freq);
  if (get_next_random_node_nullity_checker (freq, state_struct_ptr) ==
      false)
  {
    return NULL;
  }
  int rd_counter = 0;
  MarkovNodeFrequency *rel_node = (state_struct_ptr->frequencies_list);
  while (rand_getter >= 0)
  {
    rand_getter -= rel_node[rd_counter].frequency;
    if (rand_getter < 0)
    {
      return rel_node[rd_counter].markov_node;
    }
    rd_counter++;
  }
  return rel_node[rd_counter].markov_node;
}

/**
* Loops over words for a single tweet.
*/
void tweet_looper (MarkovNode *runner_node, int max_length, int
runner_tweet_len, MarkovChain *markov_chain, MarkovNode first_node)
{
  markov_chain->print_func (first_node.data);
  while (runner_tweet_len < max_length
         && (!(markov_chain->is_last (runner_node->data))))
  {
    runner_tweet_len++;
    MarkovNode *next_node = get_next_random_node (runner_node);
    markov_chain->print_func (next_node->data);
    runner_node = next_node;
  }
}

/**
* Receive markov_chain, generate and print random sentence out of it. The
* sentence must have at least 2 words in it.
* @param markov_chain
* @param first_node markov_node to start with,
*                   if NULL- choose a random markov_node
* @param  max_length maximum length of chain to generate
*/
void
generate_tweet (MarkovChain *markov_chain, MarkovNode *first_node,
                int max_length)
{
  if (first_node == NULL)
  {
    first_node = get_first_random_node (markov_chain);
  }

  bool good_start = false;
  while (good_start == false)
  {
    if (markov_chain->is_last (first_node->data) == true)
    {
      first_node = get_first_random_node (markov_chain);
    }
    else
    {
      good_start = true;
    }
  }
  tweet_looper (first_node, max_length,
                TWEET_INIT_LENGTH, markov_chain, *first_node);
}