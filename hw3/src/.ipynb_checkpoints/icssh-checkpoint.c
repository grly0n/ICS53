#include "icssh.h"
#include "helpers.h"
#include "linkedlist.h"
#include <readline/readline.h>

volatile sig_atomic_t sigchld_flag;

void SIGCHLD_handler(int sig) {
    sigchld_flag = 1;
}

int main(int argc, char* argv[]) {
    int max_bgprocs = -1;
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
    sigset_t mask_all, mask_child, mask_none;
    list_t *bg_list;
#ifdef GS
    rl_outstream = fopen("/dev/null", "w");
#endif

    // check command line arg
    if(argc > 1) {
        int check = atoi(argv[1]);
        if(check != 0)
            max_bgprocs = check;
        else {
            printf("Invalid command line argument value\n");
            exit(EXIT_FAILURE);
        }
    }

	// Setup segmentation fault handler
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    // Setup masks
    sigfillset(&mask_all);
    sigemptyset(&mask_none);
    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGCHLD);
    signal(SIGCHLD, SIGCHLD_handler);
    // create linked list of background processes
    bg_list = CreateList(list_comparator, list_printer, list_deleter);

    // print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) {
        // check if any children have terminated and reap them if necessary
        if (sigchld_flag) {
            while ((pid = waitpid(-1, NULL, 0)) > 0) {
                printf("waiting for children to terminate...\n");
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                int removeResult = removePidFromList(bg_list, pid);
                if (!removeResult) {
                    printf("ERROR REMOVING CHILD\n");
                }
                sigprocmask(SIG_SETMASK, &mask_none, NULL);
            }
            sigchld_flag = 0;
        }

        // MAGIC HAPPENS! Command string is parsed into a job struct
        // Will print out error message if command string is invalid
		job_info* job = validate_input(line);
        if (job == NULL) { // Command was empty string or invalid
			free(line);
			continue;
		}

        // continue if a process in the job had too many arguments or an argument that was too long
        if (invalid_argc_or_length(job)) {
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        //Prints out the job linked list struture for debugging
        #ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
            	debug_print_job(job);
        #endif

		// example built-in: exit
		if (strcmp(job->procs->cmd, "exit") == 0) {
			// Terminating the shell
			free(line);
			free_job(job);
            validate_input(NULL);   // calling validate_input with NULL will free the memory it has allocated
            return 0;
		}

        // built-in: cd [dir]
        if (strcmp(job->procs->cmd, "cd") == 0) {
            // if no args, cd into $HOME
            if (job->procs->argc == 1) {
                if (!chdir(getenv("HOME"))) {
                    char cwd[256];
                    getcwd(cwd, sizeof(cwd));
                    printf("%s\n", cwd);
                } else {
                    printf(DIR_ERR);
                    goto cd_end;
                }
            }
            // else, cd into the given directory
            else if (job->procs->argc == 2) {
                if (!chdir(job->procs->argv[1])) {
                    char cwd[256];
                    getcwd(cwd, sizeof(cwd));
                    printf("%s\n", cwd);
                } else {
                    printf(DIR_ERR);
                    goto cd_end;
                }
            }
            else {
                printf(DIR_ERR);
                goto cd_end;
            }
            cd_end:
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
        }

        // built-in: estatus
        if (strcmp(job->procs->cmd, "estatus") == 0) {
            if (job->procs->argc == 1) printf("%d\n", exit_status);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        // built-in: bglist
        if (strcmp(job->procs->cmd, "bglist") == 0) {
            if (job->procs->argc == 1) PrintLinkedList(bg_list, stdout);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

		sigprocmask(SIG_BLOCK, &mask_child, NULL); // Block SIGCHLD
        // example of good error handling!
        // create the child process
        if ((pid = fork()) < 0) {
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {  //If zero, then it's the child process
            sigprocmask(SIG_SETMASK, &mask_none, NULL); // Unblock SIGCHLD
			// Get first process from the job
            proc_info* proc = job->procs;
            exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd);
				
				// Cleaning up to make Valgrind happy 
				// (not necessary because child will exit. Resources will be reaped by parent)
				free_job(job);  
				free(line);
    				validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

				exit(EXIT_FAILURE);
			}
		} else { // Parent code
            sigprocmask(SIG_BLOCK, &mask_all, NULL); // block all signals
            if (job->bg) {
                // Add bg process to bg_list
                bgentry_t *bg_job = (bgentry_t*)malloc(sizeof(bgentry_t));
                bg_job->job = job;
                bg_job->pid = pid;
                bg_job->seconds = time(NULL);
                InsertInOrder(bg_list, bg_job);
            }
            sigprocmask(SIG_SETMASK, &mask_none, NULL); // unblock all signals
            // As the parent, wait for the foreground job to finish
			if (!job->bg) {
                wait_result = waitpid(pid, &exit_status, 0);
			    if (wait_result < 0) {
				    printf(WAIT_ERR);
				    exit(EXIT_FAILURE);
			    }
		    } else { // If the job is running in the background, ask for the next input
                free(line);
                continue;
            }
        }


		free_job(job);  // if a foreground job, we no longer need the data
		free(line);
	}

    // calling validate_input with NULL will free the memory it has allocated
    validate_input(NULL);

#ifndef GS
	fclose(rl_outstream);
#endif
	return 0;
}
