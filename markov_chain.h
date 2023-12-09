#ifndef _MARKOV_CHAIN_H
#define _MARKOV_CHAIN_H
#define MAX_LEN_OF_WORD 101
#define INPUT_FACTOR 1

#include "linked_list.h"
#include <stdio.h>  // For printf(), sscanf()
#include <stdlib.h> // For exit(), malloc()
#include <stdbool.h> // for bool

#define ALLOCATION_ERROR_MASSAGE "Allocation failure: "\
"Failed to allocate new memory\n"


/***************************/
/*   insert typedefs here  */
/***************************/
typedef void (*print_func) (void *);
typedef int (*comp_func) (void *, void *);
typedef void (*free_data) (void *);
typedef void *(*copy_func) (void *);
typedef bool (*is_last) (void *);

/***************************/



/***************************/
/*        STRUCTS          */
typedef struct MarkovNodeFrequency MarkovNodeFrequency;
typedef struct MarkovNode MarkovNode;
typedef struct MarkovChain MarkovChain;

/***************************/

typedef struct MarkovNode
{
    void *data;
    MarkovNodeFrequency *frequencies_list;
    int len_of_frequency_list;
    bool finisher;
} MarkovNode;

typedef struct MarkovNodeFrequency
{
    MarkovNode *markov_node;
    int frequency;
} MarkovNodeFrequency;

/* DO NOT ADD or CHANGE variable names in this struct */
typedef struct MarkovChain
{
    LinkedList *database;

    // pointer to a func that receives data from a generic type and prints it
    // returns void.
    print_func print_func;

    // pointer to a func that gets 2 pointers of generic
    // data type(same one) and compare between them */
    // returns: - a positive value if the first is bigger
    //          - a negative value if the second is bigger
    //          - 0 if equal
    comp_func comp_func;

    // a pointer to a function that gets a pointer of
    // generic data type and frees it.
    // returns void.
    free_data free_data;

    // a pointer to a function that  gets a pointer of generic data
    // type and returns a newly allocated copy of it
    // returns a generic pointer.
    copy_func copy_func;

    //  a pointer to function that gets a pointer of generic data
    //  type and returns:
    //      - true if it's the last state.
    //      - false otherwise.
    is_last is_last;
} MarkovChain;

/**
 * Get one random state from the given markov_chain's database.
 * @param markov_chain
 * @return
 */
MarkovNode *get_first_random_node (MarkovChain *markov_chain);

/**
 * Choose randomly the next state, depend on it's occurrence frequency.
 * @param state_struct_ptr MarkovNode to choose from
 * @return MarkovNode of the chosen state
 */
MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr);


/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_database (MarkovChain **markov_chain);

/**
 * Add the second markov_node to the counter list of the first markov_node.
 * If already in list, update it's counter value.
 * @param first_node
 * @param second_node
 * @param markov_chain
 * @return success/failure: true if the process was successful, false if in
 * case of allocation error.
 */
bool add_node_to_frequencies_list (MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain);

/**
* Check if data_ptr is in database. If so, return the markov_node
 * wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Pointer to the Node wrapping given state, NULL if state not in
 * database.
 */
Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr);

/**
* If data_ptr in markov_chain, return it's node. Otherwise, create new
 * node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return node wrapping given data_ptr in given chain's database
 */
Node *add_to_database (MarkovChain *markov_chain, void *data_ptr);

/**
* Returns true if markov-node's allocation had succeeded, prints allocation
* error massage and returns false otherwise.
*/
bool node_allocation_success (MarkovNode *new_node);

/**
* Creates a new markov-node with all needed properties.
*/
MarkovNode *new_node_creator (MarkovChain *markov_chain,
                              MarkovNode *new_node, void *data_ptr);

/**
* add_node_to_frequencies_list's helper, responsible for reallocating
* memory if freq list exists, allocates new memory for second node in freq
* list if freq list doesn't exist.
*/
bool frequency_list_memory_allocation (MarkovNode *first_node, MarkovNode
*second_node);

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number (int max_number);

/**
* Checks if next random node is NULL;
*/
int get_next_random_node_nullity_checker (int freq, MarkovNode
*state_struct_ptr);

/**
* Loops over words for a single tweet.
*/
void tweet_looper (MarkovNode *runner_node, int max_length, int
runner_tweet_len, MarkovChain *markov_chain, MarkovNode first_node);

/**
* Receive markov_chain, generate and print random sentence out of it. The
* sentence must have at least 2 words in it.
* @param markov_chain
* @param first_node markov_node to start with,
*                   if NULL- choose a random markov_node
* @param  max_length maximum length of chain to generate
*/
void generate_tweet (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length);

#endif /* MARKOV_CHAIN_H */
