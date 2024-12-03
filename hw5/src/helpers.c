#include "helpers.h"
#include "protocol.h"
#include <pthread.h>


stats_t curStats;
poll_t pollArray[32];
list_t *userList;
pthread_mutex_t pollArray_locks[32], curStats_lock;

// Server listening socket initialization
int server_init(unsigned int port_number) {
    struct sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Socket creation failed\n");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_number);
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char*)&opt, sizeof(opt))<0) {
        perror("setsockopt"); exit(1);
    }
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "Socket bind failure\n"); exit(1);
    }
    return sockfd;
}



// curStats global variable initialization
void curStats_init() {
    curStats.clientCnt = 0;
    curStats.threadCnt = 0;
    curStats.totalVotes = 0;
}


// pollArray initialization from poll_filename; returns number of polls initialized
int pollArray_init(char *poll_filename) {
    // Open poll_filename
    FILE *poll_file = fopen(poll_filename, "r");
    if (!poll_file) {
        fprintf(stderr, "Failed to open file %s\n", poll_filename);
        exit(2);
    }
    // Parse file line by line
    char *line = NULL, *token = NULL;
    size_t len = 0, token_len = 0;
    int poll_index = 0;
    while (getline(&line, &len, poll_file) != -1) {
        token = strtok(line, ";");      // Read poll question
        token_len = strlen(token);
        char *question = (char*)malloc((token_len+1)*sizeof(char));
        strcpy(question, token);
        poll_t currPoll;                // Create new poll_t for this poll
        currPoll.question = question;   // Assign question to poll_t
        int choices_count = atoi(strtok(NULL, ";"));    // Read choices count
        // Parse line choice by choice
        for (int i = 0; i < 4; i++) {
            choice_t currChoice;            // Create new choice_t for this choice
            if (i < choices_count) {        // Initialize non-empty choice
                token = strtok(NULL, ",");  // Read choice text
                token_len = strlen(token);
                char *choice_text = (char*)malloc((token_len+1)*sizeof(char));
                strcpy(choice_text, token);
                currChoice.text = choice_text;  // Assign text to choice_t
                currChoice.voteCnt = atoi(strtok(NULL, ";"));   // Assign voteCnt to choice_t
            } else {    // Initialize empty choice
                currChoice.text = NULL;
                currChoice.voteCnt = 0;
            }
            currPoll.options[i] = currChoice;   // Place choice in poll options
        }
        pollArray[poll_index++] = currPoll;     // Place poll in pollArray
    }
    free(line);
    fclose(poll_file);
    return poll_index;
}


// Deallocates all memory allocated by pollArray_init
void pollArray_delete(int poll_count) {
    for (int i = 0; i < poll_count; i++) {
        free(pollArray[i].question);    // Free question text
        for (int j = 0; j < 4; j++) {
            free(pollArray[i].options[j].text);  // Free choice text
        }
    }
}


// Search through the userList for a given username
// Returns the entry if found, NULL if not
user_t *SearchForUsername(list_t *list, char *username) {
    if (!list || !list->head) return NULL;
    node_t *curr = list->head;
    while (curr) {
        if (!strcmp(((user_t*)curr->data)->username, username)) return curr->data;
        curr = curr->next;
    }
    return NULL;
}


// Deletes an entry from the userList
void list_deleter(void *node) {
    user_t *user = (user_t*)((node_t*)node)->data;
    free(user->username);
    free(user);
    free(node);
}

// Writes a message to a client using the petrV protocol
void write_petrv_message(int clientfd, int code, char *msgbuf) {
    petrV_header *header = calloc(1, sizeof(petrV_header));
    header->msg_type = code;
    if (!msgbuf) header->msg_len = 0;
    else header->msg_len = strlen(msgbuf);
    wr_msg(clientfd, header, msgbuf);
    free(header);
}

// Prints results of all polls to STDOUT
void print_poll_results(int poll_count) {
   for (int i = 0; i < poll_count; i++) {
        int choices_count = 0;
        for (; choices_count < 4; choices_count++) 
            if (!pollArray[i].options[choices_count].text) break;
        printf("%s;%d", pollArray[i].question, choices_count);
        for (int j = 0; j < choices_count; j++) {
            printf(";%s,%d", pollArray[i].options[j].text, pollArray[i].options[j].voteCnt);
        }
        printf("\n");
   }
}

// Prints userList information to STDERR
void print_userList() {
    node_t *curr = userList->head;
    while (curr) {
        user_t *user = (user_t*)curr->data;
        fprintf(stderr, "%s, %d\n", user->username, user->pollVotes);
        curr = curr->next;
    }
}

// Prints curStats information to STDERR
void print_curStats() {
    fprintf(stderr, "%d, %d, %d\n", curStats.clientCnt, curStats.threadCnt, curStats.totalVotes);
}


// Returns a string containing poll information
char *get_polls_info(uint32_t pollVotes) {
    char *output = NULL;
    for (int i = 0; i < 32; i++) {
        pthread_mutex_lock(&pollArray_locks[i]);

        // Check if no more polls
        if (!pollArray[i].question) {
            pthread_mutex_unlock(&pollArray_locks[i]);
            break;
        }

        // Copy poll question
        char *pollnum = NULL, *question = NULL;
        pollnum = malloc(3*sizeof(char));
        sprintf(pollnum, "%d", i);
        question = malloc((9+strlen(pollnum)+strlen(pollArray[i].question))*sizeof(char));
        sprintf(question, "Poll %s - %s", pollnum, pollArray[i].question);
        free(pollnum);
        
        // If the poll is already voted for, do not include choices
        if ((pollVotes & (1 << i)) != 0) {
            char *temp = NULL;
            if (output) {
                temp = malloc((2+strlen(output)+strlen(question))*sizeof(char));
                sprintf(temp, "%s%s\n", output, question);
                free(output);
            } else {
                temp = malloc((2+strlen(question))*sizeof(char));
                sprintf(temp, "%s\n", question);
            }
            free(question);
            output = temp;
            pthread_mutex_unlock(&pollArray_locks[i]);
            continue;
        }

        // If including choices, add " - " separator
        char *question2 = malloc(4+(strlen(question))*sizeof(char));
        strcpy(question2, question);
        strcat(question2, " - ");
        free(question);

        // Copy choices
        char *choices = NULL;
        for (int j = 0; j < 4; j++) {
            // Check if no more choices
            if (!pollArray[i].options[j].text) break;
            
            // Copy choice number and text
            char *choicenum = malloc(2*sizeof(char));
            sprintf(choicenum, "%d", j);
            char *choicetext = NULL;
            if (j) {
                choicetext = malloc((4+strlen(choicenum)+strlen(pollArray[i].options[j].text))*sizeof(char));
                sprintf(choicetext, ", %s:%s", choicenum, pollArray[i].options[j].text);
            }
            else {
                choicetext = malloc((2+strlen(choicenum)+strlen(pollArray[i].options[j].text))*sizeof(char));
                sprintf(choicetext, "%s:%s", choicenum, pollArray[i].options[j].text);
            }
            free(choicenum);

            // Add choice to choices string
            char *temp = NULL;
            if (choices) {
                temp = malloc((1+strlen(choices)+strlen(choicetext))*sizeof(char));
                strcpy(temp, choices);
                strcat(temp, choicetext);
                free(choices);
            } else {
                temp = malloc((1+strlen(choicetext))*sizeof(char));
                strcpy(temp, choicetext);
            }
            free(choicetext);
            choices = temp;
        }

        // Add question and choices to output
        char *temp = NULL;
        if (output) {
            temp = malloc((2+strlen(output)+strlen(question2)+strlen(choices))*sizeof(char));
            sprintf(temp, "%s%s%s\n", output, question2, choices);
            free(output);
        } else {
            temp = malloc((2+strlen(question2)+strlen(choices))*sizeof(char));
            sprintf(temp, "%s%s\n", question2, choices);
        }
        output = temp;
        free(question2);
        free(choices);
        pthread_mutex_unlock(&pollArray_locks[i]);
    }
    return output;
}

// Votes on a poll given a poll number and option number
// Returns 0 on success
// Returns 1 if given poll does not exist
// Returns 2 if given option does not exist
// Returns 3 if the user has already voted on the given poll
int vote_on_poll(int poll_number, int option_number, uint32_t *pollVotes) {
    // Out-of-bounds error checking
    if (poll_number > 31) return 1;
    if (option_number > 3) return 2;

    pthread_mutex_lock(&pollArray_locks[poll_number]);      // pollArray MUTEX BEGIN
    // If poll does not exist, exit with error
    if (!pollArray[poll_number].question) {
        pthread_mutex_unlock(&pollArray_locks[poll_number]);
        return 1;
    }

    // If option does not exist, exit with error
    if (!pollArray[poll_number].options[option_number].text) {
        pthread_mutex_unlock(&pollArray_locks[poll_number]);
        return 2;
    }

    // If poll has already been voted for, exit with error
    if (*pollVotes & (1 << poll_number)) {
        pthread_mutex_unlock(&pollArray_locks[poll_number]);
        return 3;
    }

    // Increment vote count of given option
    pollArray[poll_number].options[option_number].voteCnt++;
    // Increment curStats total vote count
    pthread_mutex_lock(&curStats_lock);                     // curStats MUTEX BEGIN
    curStats.totalVotes++;
    pthread_mutex_unlock(&curStats_lock);                   // curStats MUTEX END
    // Update votes bitvector
    *pollVotes = *pollVotes | (1 << poll_number);
    pthread_mutex_unlock(&pollArray_locks[poll_number]);    // pollArray MUTEX END
    return 0;
}


// Fills a string buffer with statistics for selected poll(s)
// Returns string on success
// Returns NULL for EPDENIED (no polls voted on, given poll not voted on)
char *get_polls_stats(uint32_t pollVotes, int poll_num) {
    // If user hasn't voted on any polls, exit with error
    if (!pollVotes) return NULL;
    // Out-of-bounds checking
    if (poll_num > 31) return NULL;
    
    // Single poll handling
    char *output = NULL;
    if (poll_num != -1) {
        pthread_mutex_lock(&pollArray_locks[poll_num]);         // pollArray MUTEX BEGIN
        // If given poll number is invalid, exit with error
        if (!pollArray[poll_num].question) {
            pthread_mutex_unlock(&pollArray_locks[poll_num]);
            return NULL;
        }

        // If the given poll number has not been voted for, exit with error
        if (!(pollVotes & (1 << poll_num))) {
            pthread_mutex_unlock(&pollArray_locks[poll_num]);
            return NULL;
        }

        // Create poll header ("Poll <poll_num> - ")
        char *poll_header = NULL;
        if (poll_num > 9) poll_header = malloc(10*sizeof(char));
        else poll_header = malloc(11*sizeof(char));
        sprintf(poll_header, "Poll %d - ", poll_num);

        // Create poll choices "<choice_text>:<choice_cnt>" or ", <choice_text>:<choice_cnt>"
        char *choices = NULL;
        for (int i = 0; i < 4; i++) {
            if (!pollArray[poll_num].options[i].text) break;    // If no more choices, break

            // Convert count to string (max size: 10 digits + 1 null byte)
            char *choice_count = malloc(11*sizeof(char));
            sprintf(choice_count, "%d", pollArray[poll_num].options[i].voteCnt);
            
            // Create individual choice string
            char *choice = NULL;
            if (i) {    // Format: ", <text>:<count>"
                choice = malloc((3+strlen(pollArray[poll_num].options[i].text)+strlen(choice_count))*sizeof(char));
                sprintf(choice, ",%s:%s", pollArray[poll_num].options[i].text, choice_count);
            } else {    // Format: "<text>:<count>"
                choice = malloc((2+strlen(pollArray[poll_num].options[i].text)+strlen(choice_count))*sizeof(char));
                sprintf(choice, "%s:%s", pollArray[poll_num].options[i].text, choice_count);
            }
            free(choice_count);

            // Add choice to choices string
            char *temp = NULL;
            if (choices) {
                temp = malloc((1+strlen(choices)+strlen(choice))*sizeof(char));
                strcpy(temp, choices);
                strcat(temp, choice);
                free(choices);
            } else {
                temp = malloc((1+strlen(choice))*sizeof(char));
                strcpy(temp, choice);
            }
            choices = temp;
            free(choice);
        }
        // Create output: header + choices
        output = malloc((2+strlen(poll_header)+strlen(choices))*sizeof(char));
        sprintf(output, "%s%s\n", poll_header, choices);
        free(poll_header);
        free(choices);
        pthread_mutex_unlock(&pollArray_locks[poll_num]);   // pollArray MUTEX END
    }
    // All poll handling
    else if (poll_num == -1) {
        for (int i = 0; i < 32; i++) {
            pthread_mutex_lock(&pollArray_locks[i]);        // pollArray MUTEX START
            // Check if no more polls
            if (!pollArray[i].question) {
                pthread_mutex_unlock(&pollArray_locks[i]);
                break;
            }

            // Check if poll has not been voted on
            if (!(pollVotes & (1 << i))) {
                pthread_mutex_unlock(&pollArray_locks[i]);
                continue;
            }

            // Create poll header ("Poll <poll_num> - ")
            char *poll_header = NULL;
            if (i > 9) poll_header = malloc(10*sizeof(char));
            else poll_header = malloc(11*sizeof(char));
            sprintf(poll_header, "Poll %d - ", i);

            // Create poll choices "<choice_text>:<choice_cnt>" or ", <choice_text>:<choice_cnt>"
            char *choices = NULL;
            for (int j = 0; j < 4; j++) {
                if (!pollArray[i].options[j].text) break;    // If no more choices, break

                // Convert count to string (max size: 10 digits + 1 null byte)
                char *choice_count = malloc(11*sizeof(char));
                sprintf(choice_count, "%d", pollArray[i].options[j].voteCnt);
                
                // Create individual choice string
                char *choice = NULL;
                if (j) {    // Format: ", <text>:<count>"
                    choice = malloc((3+strlen(pollArray[i].options[j].text)+strlen(choice_count))*sizeof(char));
                    sprintf(choice, ",%s:%s", pollArray[i].options[j].text, choice_count);
                } else {    // Format: "<text>:<count>"
                    choice = malloc((2+strlen(pollArray[i].options[j].text)+strlen(choice_count))*sizeof(char));
                    sprintf(choice, "%s:%s", pollArray[i].options[j].text, choice_count);
                }
                free(choice_count);

                // Add choice to choices string
                char *temp = NULL;
                if (choices) {
                    temp = malloc((1+strlen(choices)+strlen(choice))*sizeof(char));
                    strcpy(temp, choices);
                    strcat(temp, choice);
                    free(choices);
                } else {
                    temp = malloc((1+strlen(choice))*sizeof(char));
                    strcpy(temp, choice);
                }
                choices = temp;
                free(choice);
            }
            // Append to output
            char *temp = NULL;
            if (output) {
                temp = malloc((2+strlen(output)+strlen(poll_header)+strlen(choices))*sizeof(char));
                sprintf(temp, "%s%s%s\n", output, poll_header, choices);
                free(output);
            } else {
                temp = malloc((2+strlen(poll_header)+strlen(choices))*sizeof(char));
                sprintf(temp, "%s%s\n", poll_header, choices);
            }
            free(poll_header);
            free(choices);
            output = temp;
            pthread_mutex_unlock(&pollArray_locks[i]);          // pollArray MUTEX END
        }
    }
    return output;
}

