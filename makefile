CC = gcc
CFLAGS = -Wall -Wextra -Wvla -std=c99

tweets: markov_chain.h markov_chain.c linked_list.h linked_list.c tweets_generator.c
	$(CC) $(CFLAGS) tweets_generator.c markov_chain.c linked_list.c -o tweets_generator

snake: markov_chain.h markov_chain.c linked_list.h linked_list.c snakes_and_ladders.c
	$(CC) $(CFLAGS) snakes_and_ladders.c markov_chain.c linked_list.c -o snakes_and_ladders