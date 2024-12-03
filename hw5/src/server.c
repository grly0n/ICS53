#include "server.h"
#include "protocol.h"
#include "linkedlist.h"
#include "helpers.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>

// PetrV protocol normal code macros
#define OK 0x00
#define LOGIN 0x1
#define LOGOUT 0x2
#define PLIST 0x3
#define STATS 0x4
#define VOTE 0x5

// PetrV protocol error code macros
#define EUSERLOGIN 0xF0
#define EPDENIED 0xF1
#define EPNOTFOUND 0xF2
#define ECNOTFOUND 0xF3
#define ESERV 0xFF

// Synchronization locks
pthread_mutex_t userList_lock, log_lock;
extern pthread_mutex_t pollArray_locks[32];
extern pthread_mutex_t curStats_lock;
// Sigint flag
volatile int sigint_flag = 0;
// Voting log file
FILE *log_file = NULL;


// Synchronization locks initialization
void synchro_locks_init() {
    pthread_mutex_init(&curStats_lock, NULL);
    pthread_mutex_init(&userList_lock, NULL);
    pthread_mutex_init(&log_lock, NULL);
    for (int i = 0; i < 32; i++) pthread_mutex_init(&pollArray_locks[i], NULL);
}


// Sigint handler
static void sigint_handler(int signo) {sigint_flag = 1;}



// Process client thread body
void *process_client(void *user) {
    pthread_detach(pthread_self());
    pthread_mutex_lock(&userList_lock);     // userList MUTEX BEGIN
    user_t *user_info = (user_t*)user;
    char *username = user_info->username;   // Get client username
    int clientfd = user_info->socket_fd;    // Get client file descriptor
    uint32_t votes = user_info->pollVotes;       // Get client pollVotes count
    pthread_mutex_unlock(&userList_lock);   // userList MUTEX END
    
    while(1) {
        // Check if SIGINT has occurred
        if (sigint_flag) {
            goto onClientLOGOUT;
        }
        // Read message from client
        petrV_header header;
        if (rd_msgheader(clientfd, &header) == -1) {
            if (sigint_flag) {
                goto onClientLOGOUT;   
            } else {
                fprintf(stderr, "Client rd_msgheader() error\n");
                continue;
            }
        }

        // LOGOUT handling
        if (header.msg_type == LOGOUT) {
            write_petrv_message(clientfd, OK, NULL);
            pthread_detach(pthread_self());
            goto onClientLOGOUT;
        }

        // PLIST handling
        else if (header.msg_type == PLIST) {
            char *polls_info = get_polls_info(votes);   // Get pollArray info
            write_petrv_message(clientfd, PLIST, polls_info);   // Write PLIST with content to client
            pthread_mutex_lock(&log_lock);              // log_file MUTEX BEGIN
            fprintf(log_file, "%s PLIST\n", username);  // Write PLIST event to log
            pthread_mutex_unlock(&log_lock);            // log_file MUTEX END
            free(polls_info);
            continue;
        }

        // STATS handling
        else if (header.msg_type == STATS) {
            char buffer[header.msg_len];                // Read information from client
            read(clientfd, buffer, sizeof(buffer));
            int poll_index = atoi(buffer);
            char *polls_stats = get_polls_stats(votes, poll_index);
            if (polls_stats) {    // On stats success
                write_petrv_message(clientfd, STATS, polls_stats);
                free(polls_stats);
                pthread_mutex_lock(&log_lock);              // log_file MUTEX BEGIN
                fprintf(log_file, "%s STATS %d\n", username, votes);    // Write STATS event to log
                pthread_mutex_unlock(&log_lock);            // log_file MUTEX END
            } else {        // On invalid poll
                write_petrv_message(clientfd, EPDENIED, NULL);
            }
            continue;
        }

        // VOTE handling
        else if (header.msg_type == VOTE) {
            char buffer[header.msg_len];                // Read information from client
            read(clientfd, buffer, sizeof(buffer));
            int poll_num = atoi(strtok(buffer, " "));
            int choice_num = atoi(strtok(NULL, " "));
            int vote_status = vote_on_poll(poll_num, choice_num, &votes);
            // On vote success
            if (!vote_status) {
                write_petrv_message(clientfd, OK, NULL);
                pthread_mutex_lock(&log_lock);              // log_file MUTEX BEGIN
                fprintf(log_file, "%s VOTE %d %d %d\n", username, poll_num, choice_num, votes);
                pthread_mutex_unlock(&log_lock);            // log_file MUTEX END
            } // On invalid poll number
            else if (vote_status == 1) {
                write_petrv_message(clientfd, EPNOTFOUND, NULL);
            } // On invalid choice number
            else if (vote_status == 2) {
                write_petrv_message(clientfd, ECNOTFOUND, NULL);
            } // On already voted-for poll
            else {
                write_petrv_message(clientfd, EPDENIED, NULL);
            }
            continue;
        }
    }
    
    onClientLOGOUT:
        pthread_mutex_lock(&log_lock);          // log_file MUTEX BEGIN
        fprintf(log_file, "%s LOGOUT\n", username);       // Print logout info to log
        pthread_mutex_unlock(&log_lock);        // log_file MUTEX END
        pthread_mutex_lock(&userList_lock);     // userList MUTEX BEGIN
        user_info->socket_fd = -1;              // Set socket_fd to -1
        user_info->pollVotes = votes;           // Update pollVotes value
        pthread_mutex_unlock(&userList_lock);   // userList MUTEX END
        close(clientfd);
        //pthread_exit(NULL);
        return NULL;
}



// Main server body
int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, USAGE_MSG);
                exit(EXIT_FAILURE);
        }
    }

    // 3 positional arguments necessary
    if (argc != 4) {
        fprintf(stderr, USAGE_MSG);
        exit(EXIT_FAILURE);
    }
    unsigned int port_number = atoi(argv[1]);
    char *poll_filename = argv[2];
    char *log_filename = argv[3];

    // Server Start-up
    int sockfd = server_init(port_number);  // Socket creation
    curStats_init();                        // Server statistics initialization
    int poll_count = pollArray_init(poll_filename);     // Poll array initialization
    log_file = fopen(log_filename, "w+");           // Open log file
    if (!log_file) {fprintf(stderr, "Failed to open file %s\n", log_filename); exit(2);}
    userList = CreateList(list_deleter);    // User list initialization
    synchro_locks_init();                   // Synchronization locks initialization
    struct sigaction myaction = {{0}};      // Sigint handler initialization
    myaction.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &myaction, NULL) == -1) fprintf(stderr, "SIGINT handler install failure\n");
    printf("Server initialized with %d polls.\nCurrently listening on port %d.\n", poll_count, port_number);

    // Start listening for incoming client connection
    if ((listen(sockfd, 1)) != 0) {
        fprintf(stderr, "Listen failure\n"); exit(EXIT_FAILURE);
    }

    //int clientfd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    //pthread_t tid;

    // Server main thread
    while(1) {
        // Check sigint flag
        if (sigint_flag) {goto onSIGINT;}

        // Accept client connection
        int *clientfd = malloc(sizeof(int));
        *clientfd = accept(sockfd, (SA*)&client_addr, &client_addr_len);
        if (*clientfd < 0) {
            if (sigint_flag) {  // SIGINT handling
                free(clientfd);
                goto onSIGINT;    
            } else {            // Normal interrupt handling
                fprintf(stderr, "Server accept failure\n");
                exit(EXIT_FAILURE);
            }
        }

        // Receive header of first message from client
        petrV_header header;
        if (rd_msgheader(*clientfd, &header) == -1) {
            fprintf(stderr, "rd_msgheader failure\n");
            close(*clientfd);
            free(clientfd);
            continue;
        }

        // Check if header is of type LOGIN
        if (header.msg_type != LOGIN) {
            fprintf(stderr, "First message not of LOGIN, terminating connection\n");
            close(*clientfd);
            free(clientfd);
            continue;
        }
        
        // Update client count
        pthread_mutex_lock(&curStats_lock);
        curStats.clientCnt++;
        pthread_mutex_unlock(&curStats_lock);

        // Read username from client
        char buffer[header.msg_len];
        int received_size = read(*clientfd, buffer, sizeof(buffer));
        if (received_size < 0) {
            fprintf(stderr, "Read from client failure\n");
            close(*clientfd);
            free(clientfd);
            continue;
        }
        
        // Check username for spaces
        else {
            int i = 0;
            for (; i < received_size; i++) {
                if (buffer[i] == ' ') {
                    pthread_mutex_lock(&log_lock);      // log_file MUTEX BEGIN
                    fprintf(log_file, "REJECTED %s\n", buffer);
                    pthread_mutex_unlock(&log_lock);    // log_file MUTEX END
                    write_petrv_message(*clientfd, ESERV, NULL);
                    close(*clientfd);
                    free(clientfd);
                    break;
                }
            }
            if (i < received_size) continue;
        }

        // Search userList for username
        pthread_mutex_lock(&userList_lock);             // userList MUTEX BEGIN
        pthread_mutex_lock(&log_lock);                  // log_file MUTEX BEGIN
        user_t *search_result = SearchForUsername(userList, buffer);
        if (!search_result) {                           // Create new instance of user_t upon first login
            user_t *newUser = malloc(sizeof(user_t));
            newUser->username = malloc((strlen(buffer)+1)*sizeof(char));
            strcpy(newUser->username, buffer);
            newUser->socket_fd = *clientfd;
            newUser->pollVotes = 0;
            InsertAtHead(userList, newUser);
            search_result = newUser;
            fprintf(log_file, "CONNECTED %s\n", newUser->username);
            write_petrv_message(*clientfd, OK, NULL);
            free(clientfd);
        } else {                                        // Update existing user_t instance
            if (search_result->socket_fd != -1) {       // If the client is already logged in, reject the new connection
                fprintf(log_file, "REJECTED %s\n", search_result->username);
                write_petrv_message(*clientfd, EUSRLGDIN, NULL);
                close(*clientfd);                       // Close client connection
                free(clientfd);
                pthread_mutex_unlock(&log_lock);        // log_file MUTEX END
                pthread_mutex_unlock(&userList_lock);   // userList MUTEX END
                continue;
            } else {                                    // If client is not logged in, update the user_t information
                search_result->socket_fd = *clientfd;
                fprintf(log_file, "RECONNECTED %s\n", search_result->username);
                write_petrv_message(*clientfd, OK, NULL);
                free(clientfd);
            }
        }
        pthread_mutex_unlock(&log_lock);        // log_file MUTEX END

        // Client thread spawning
        pthread_t tid;
        pthread_create(&tid, NULL, process_client, (void*)search_result);
        search_result->tid = tid;               // Update user_t tid
        pthread_mutex_unlock(&userList_lock);   // userList MUTEX END
        pthread_mutex_lock(&curStats_lock);     // curStats MUTEX START
        curStats.threadCnt++;                   // Update curStats thread count
        pthread_mutex_unlock(&curStats_lock);   // curStats MUTEX END     
    }
    onSIGINT:
        close(sockfd);      // Close listening socket
        
        pthread_mutex_lock(&userList_lock);     // userList MUTEX START
        node_t *curr = userList->head;
        while (curr) {      // Send SIGINT to all active threads
            pthread_t tid = ((user_t*)curr->data)->tid;
            pthread_mutex_unlock(&userList_lock);   // userList MUTEX END
            if (tid != -1) { 
                pthread_kill(tid, SIGINT);
                pthread_join(tid, NULL);
            }
            pthread_mutex_lock(&userList_lock);     // userList MUTEX START
            curr = curr->next;
        }
        pthread_mutex_unlock(&userList_lock);   // userList MUTEX END

        // No more mutexes are required after this point (only running thread is main server)
        print_poll_results(poll_count); // Print final poll results (STDOUT)
        print_userList();               // Print users in userList  (STDERR)
        print_curStats();               // Print final server stats (STDERR)
        DeleteList(userList);           // Delete userList
        pollArray_delete(poll_count);   // Delete dynamically allocated pollArray contents
        fclose(log_file);               // Close log_file
        return 0;
}
