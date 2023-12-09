cmake_minimum_required(VERSION 3.24)
project(ex3b_adishnaidman C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(ex3b_adishnaidman
        linked_list.c
        linked_list.h
        markov_chain.h
        markov_chain.c
        tweets_generator.c)
