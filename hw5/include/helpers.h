#ifndef HELPERS_H
#define HELPERS_H

#include "server.h"

// Server port initialization
int server_init(unsigned int port_number);

// Poll array initialization from file
int pollArray_init(char *poll_filename);

// Current stats initialization (all 0)
void curStats_init();

// Poll array deletion of dynamically allocated memory
void pollArray_delete(int poll_count);

// Searches and returns user_t instance containing given username
user_t *SearchForUsername(list_t *list, char *username);

// Deletes a node in the user list
void list_deleter(void *node);

// Writes a message using the petrV protocol to a file descriptor
void write_petrv_message(int clientfd, int code, char *msgbuf);

// Print results of all polls to STDOUT
void print_poll_results(int poll_count);

// Both printed to STDERR (called when server receives SIGINT)
void print_userList();
void print_curStats();

// Returns a string containing all information about polls
char *get_polls_info(uint32_t pollVotes);

// Votes for a poll given a poll number and option number
int vote_on_poll(int poll_number, int option_number, uint32_t *pollVotes);

// Fills a string buffer with all statistics about selected poll(s)
char *get_polls_stats(uint32_t pollVotes, int poll_num);

#endif
