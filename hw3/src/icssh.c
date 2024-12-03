#include "icssh.h"
#include "helpers.h"
#include "linkedlist.h"
#include <readline/readline.h>

volatile sig_atomic_t sigchld_flag = 0;

void SIGCHLD_handler(int sig) {
    sigchld_flag = 1;
}

void SIGUSR2_handler(int sig) {
    time_t seconds = time(NULL);
    char *info = ctime(&seconds);
    write(2, info, strlen(info)*sizeof(char));
}

int main(int argc, char* argv[]) {
    int max_bgprocs = -1, curr_bgprocs = 0;
	int running_piped_procs = 0;
    int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char *line;
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

    // Setup SIGCHLD handler
    if (signal(SIGCHLD, SIGCHLD_handler) == SIG_ERR) {
        perror("Failed to set SIGCHLD handler");
        exit(EXIT_FAILURE);
    }

    // Setup SIGUSR2 handler
    if (signal(SIGUSR2, SIGUSR2_handler) == SIG_ERR) {
        perror("Failed to set SIGUSR2 handler");
        exit(EXIT_FAILURE);
    }

    // Setup masks
    sigfillset(&mask_all);
    sigemptyset(&mask_none);
    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGCHLD);
    // create linked list of background processes
    bg_list = CreateList(list_comparator, list_printer, list_deleter);
   
    exit_status = 0;
    // print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) {
        // check if any children have terminated and reap them if necessary
        if (sigchld_flag) {
            while ((pid = waitpid(-1, &exit_status, WNOHANG)) > 0) {
                sigprocmask(SIG_BLOCK, &mask_all, NULL);            // Block all signals
                int removeResult = removePidFromList(bg_list, pid, 1); // Remove terminated child from bg_list
                if (!removeResult) {
                    printf("ERROR REMOVING CHILD\n");
                } else curr_bgprocs--;
                sigprocmask(SIG_SETMASK, &mask_none, NULL);         // Unblock all signals
            }
            sigchld_flag = 0;   // Reset SIGCHLD flag
        }

        // MAGIC HAPPENS! Command string is parsed into a job struct
        // Will print out error message if command string is invalid
		job_info* job = validate_input(line);
        if (job == NULL) { // Command was empty string or invalid
			free(line);
			continue;
		}

        // Process in the job had too many arguments or an argument that was too long
        if (invalid_argc_or_length(job)) {
            fprintf(stderr, EXEC_ERR, job->line);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        // Background command when max_bgprocs is reached
        if (job->bg && max_bgprocs > -1 && curr_bgprocs == max_bgprocs) {
            fprintf(stderr, BG_ERR);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        //Prints out the job linked list struture for debugging
        #ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
            	debug_print_job(job);
        #endif
        
        // ====================================BUILT IN COMMANDS=======================================

		// example built-in: exit
		if (strcmp(job->procs->cmd, "exit") == 0) {
			// Killing all running background jobs
            node_t *curr = bg_list->head;
            while (curr) {
                bgentry_t *bg_job = (bgentry_t*)curr->data;
                kill(bg_job->pid, 9);
                curr_bgprocs--;
                removePidFromList(bg_list, bg_job->pid, 1);
                curr = bg_list->head;
            }
            // Terminating the shell
			free(line);
			free_job(job);
            validate_input(NULL);   // calling validate_input with NULL will free the memory it has allocated
            free(bg_list);
            return 0;
		}

        // built-in: cd [dir]
        if (strcmp(job->procs->cmd, "cd") == 0) {
            // if no args, cd into $HOME
            if (job->procs->argc == 1) {
                if (!chdir(getenv("HOME"))) {
                    char cwd[256];
                    getcwd(cwd, sizeof(cwd));
                    fprintf(stdout, "%s\n", cwd);
                } else {
                    fprintf(stderr, DIR_ERR);
                    goto cd_end;
                }
            }
            // else, cd into the given directory
            else {
                if (!chdir(job->procs->argv[1])) {
                    char cwd[256];
                    getcwd(cwd, sizeof(cwd));
                    fprintf(stdout, "%s\n", cwd);
                } else {
                    fprintf(stderr, DIR_ERR);
                    goto cd_end;
                }
            }
            cd_end:
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
        }

        // built-in: estatus
        if (strcmp(job->procs->cmd, "estatus") == 0) {
            printf("%d\n", exit_status);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        // built-in: bglist
        if (strcmp(job->procs->cmd, "bglist") == 0) {
            PrintLinkedList(bg_list, stderr);
            free(line);
            free_job(job);
            validate_input(NULL);
            continue;
        }

        // built-in: fg / fg <pid>
        if (strcmp(job->procs->cmd, "fg") == 0) {
            if (!bg_list->length) {
                fprintf(stderr, PID_ERR);
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
            }
            if (job->procs->argc == 1) { // fg
                pid_t pid, proc_pid;
                // mask all signals
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                // get pid to bring to foreground (most recently started)
                proc_pid = ((bgentry_t*)bg_list->head->data)->pid;
                if (bg_list->head) printf("%s\n", ((bgentry_t*)bg_list->head->data)->job->line);
                // remove from list of background processes
                int removeResult = removePidFromList(bg_list, ((bgentry_t*)bg_list->head->data)->pid, 0);
                if (!removeResult) {fprintf(stderr, PID_ERR); goto fg_end;}
                // decrement number of background procsses
                else curr_bgprocs--;
                // unmask all signals
                sigprocmask(SIG_SETMASK, &mask_none, NULL);
                // wait for the process to terminate in the foreground
                while ((pid = waitpid(proc_pid, &exit_status, 0)) > 0);
                if (WIFEXITED(exit_status)) exit_status = WEXITSTATUS(exit_status);
            } else { // fg <pid>
                // get pid to bring to foreground (user-specified)
                pid_t pid = atoi(job->procs->argv[1]);
                // check if given pid is running
                node_t *node = listContainsPid(bg_list, pid);
                if (!node) {fprintf(stderr, PID_ERR); goto fg_end;}
                else printf("%s\n", ((bgentry_t*)node->data)->job->line);
                // mask all signals
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                // remove from list of background processes
                int removeResult = removePidFromList(bg_list, pid, 0);
                if (!removeResult) {printf(PID_ERR); goto fg_end;}
                // decrement number of background processes
                else curr_bgprocs--;
                // unmask all signals
                sigprocmask(SIG_SETMASK, &mask_none, NULL);
                // wait for the process to terminate in the foreground
                while ((pid = waitpid(pid, &exit_status, 0)) > 0);
                if (WIFEXITED(exit_status)) exit_status = WEXITSTATUS(exit_status);
            }
            // free memory used
            fg_end:
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
        }

        if (strcmp(job->procs->cmd, "ascii53") == 0) {
            printf(
"      #######                                                                                       \n"
"   ##-::::::-#                    ####                                                              \n"
"  ##::::::::::#                ###########                                                          \n"
" ##:::::::::::*#             ###############           ##   ##=##                                   \n"
" #:::::::::::::##          ###################        ##:####:::#                                   \n"
"##:::::::::::::##         ######################      #:::##::::*#                                  \n"
"#:::::::::::::::#       ##########################   ##:::-#:##::#                                  \n"
"#:::::::::::::::##     ############################  #:=#::#* #=:#                                  \n"
"#::::::::::::::::##  ###############    .##+ .########:#+#:#   #-#                                  \n"
"#::::::::::::::::::=*######=##-  ##  ## ###=   #####*+:#:=##                                        \n"
"#+::::::::::::::::::::####  ###  ##  ######=   ####:::::::::#                                       \n"
"##:::::::::::::::::::::#### ###  *#  ######*  .###:::::###*:::-###                                  \n"
" #::::::::::::::::::::::+##. ##  *#- =## ###  ###::::::#   +#*::::-###                              \n"
" ##:::=:::::::::::::::::::##     *##  ## ###  ##::::::::#*  .##+::::::=###                          \n"
"  #-::#::::::::::::::::::::##:   ####   .### ##*::::::::::-####:::::::::::-###                      \n"
"   #::*#::*:::::::::::::::::###################::::::::::::::::::::::::::::::::####                 \n"
"    #=:#=:#::-:::::::::::::::#################:::::::::+:::::::::::::::::::::::::::-####            \n"
"     ## #+##:#+:::::::::::::::################:::::::::::*##*=::::::::::::::::::::::::::-#####      \n"
"          # # #+:#*:::::::::::+##############::::::::::#-#           #####*::::::::::::::::::::+### \n"
"                ##-::::::::::::##############:::::::::#:#                    ###################=:-#\n"
"                ##::::::::::::+##          #:::::::::#::#                                        ## \n"
"               ##::::::::::::-#:#          #::::::::-#:*#                                           \n"
"               #:::::::::::=#+:-#         ##::::::::#::##                                           \n"
"              #*::::::::##=::::#          #-:::::::-#::+######                                      \n"
"             ##::::::::#+:::::####        #::::::::=#::::::::+#                                     \n"
"            ##:::::::::##===-::::##       #::::::::::####=:::*#                                     \n"
"            #=::::::::::::::-#####       ##:::::::::::::::###                                       \n"
"            #::::::::::::::::##           ##*+*########*::##                                        \n"
"             ########   ######                                                                      \n");                  
             free(line);
             free_job(job);
             validate_input(NULL);
             continue;
        }
        // ====================================FORKING PROCESS=======================================

        // Variables for in and out files
        int in_fd = 0, out_fd = 0;

        // Check if input and output are the same file
        if (job->in_file && job->out_file) {
            if (!strcmp(job->in_file, job->out_file)) {
                fprintf(stderr, RD_ERR);
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
            }
        }

        // Open input file
		if (job->in_file) {
            in_fd = open(job->in_file, O_RDWR);
            if (in_fd < 0) {
                fprintf(stderr, RD_ERR);
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
            }
        }
        // Open output file
        if (job->out_file) {
            out_fd = open(job->out_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
            if (out_fd < 0) {
                fprintf(stderr, RD_ERR);
                free(line);
                free_job(job);
                validate_input(NULL);
                continue;
            }
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
            // Duplicate infile to stdin
            if (in_fd) {
                int dup_status = dup2(in_fd, 0);
                if (dup_status < 0) {
                    perror("Error duping input to stdin");
                    exit(EXIT_FAILURE);
                }
                int close_status = close(in_fd);
                if (close_status < 0) {
                    perror("Error closing input file");
                    exit(EXIT_FAILURE);
                }
            }
            // Duplicate outfile to stdout
            if (out_fd) {
                int dup_status = dup2(out_fd, 1);
                if (dup_status < 0) {
                    perror("Error duping input to stdout");
                    exit(EXIT_FAILURE);
                }
                int close_status = close(out_fd);
                if (close_status < 0) {
                    perror("Error closing output file");
                    exit(EXIT_FAILURE);
                }
            }
            // Get first process from the job
            proc_info *proc = job->procs;
            // Piped process handling
            if (job->nproc > 1) {
                // Case for two piped processes
                if (job->nproc == 2) {
                    // create one pipe
                    int p1[2];
                    pipe(p1);
                    proc_info *lproc = proc, *rproc = proc->next_proc;
                    if (fork() == 0) {
                        // Pipe setup
                        close(p1[1]);
                        dup2(p1[0], 0);
                        close(p1[0]);
                        // Error redirection setup
                        /*
                        if (rproc->err_file) {
                            if ((job->in_file && !strcmp(rproc->err_file, job->in_file)) || (job->out_file && !strcmp(rproc->err_file, job->out_file))) {
                                fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE); }
                            int err_fd = open(rproc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                            if (err_fd < 0) {fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE);}
                            int dup_status = dup2(err_fd, 2);
                            if (dup_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                            int close_status = close(err_fd);
                            if (close_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                        }
                        */
                        // Execute command
                        execvp(rproc->cmd, rproc->argv);
                    } else {
                        close(p1[0]);
                        dup2(p1[1], 1);
                        close(p1[1]);
                        // Error redirection setup
                        /*
                        if (lproc->err_file) {
                            if ((job->in_file && !strcmp(lproc->err_file, job->in_file)) || (job->out_file && !strcmp(lproc->err_file, job->out_file))) {
                                fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE); }
                            int err_fd = open(lproc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                            if (err_fd < 0) {fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE);}
                            int dup_status = dup2(err_fd, 2);
                            if (dup_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                            int close_status = close(err_fd);
                            if (close_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                        }
                        */
                        // Execute command
                        execvp(lproc->cmd, lproc->argv);
                    }
                // Case for three piped processes
                } else if (job->nproc == 3) {
                    // create two pipes
                    int p1[2], p2[2];
                    pipe(p1);
                    pipe(p2);
                    proc_info *lproc = proc, *mproc = proc->next_proc, *rproc = proc->next_proc->next_proc;
                    if (fork() == 0) {
                        // Pipe setup
                        close(p2[1]);
                        close(p1[0]);
                        close(p1[1]);
                        dup2(p2[0], 0);
                        close(p2[0]);
                        // Error redirection setup
                        /*
                        if (rproc->err_file) {
                            if ((job->in_file && !strcmp(rproc->err_file, job->in_file)) || (job->out_file && !strcmp(rproc->err_file, job->out_file))) {
                                fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE); }
                            int err_fd = open(rproc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                            if (err_fd < 0) {fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE);}
                            int dup_status = dup2(err_fd, 2);
                            if (dup_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                            int close_status = close(err_fd);
                            if (close_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                        }
                        */
                        // Execute command
                        execvp(rproc->cmd, rproc->argv);
                    } else {
                        if (fork() == 0) {
                            // Pipe setup
                            close(p1[1]);
                            close(p2[0]);
                            dup2(p1[0], 0);
                            dup2(p2[1], 1);
                            close(p1[0]);
                            close(p2[0]);
                            // Error redirection setup
                            /*
                            if (mproc->err_file) {
                                if ((job->in_file && !strcmp(mproc->err_file, job->in_file)) || (job->out_file && !strcmp(mproc->err_file, job->out_file))) {
                                    fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE); }
                                int err_fd = open(mproc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                                if (err_fd < 0) {fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE);}
                                int dup_status = dup2(err_fd, 2);
                                if (dup_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                                int close_status = close(err_fd);
                                if (close_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                            }
                            */
                            // Execute command
                            execvp(mproc->cmd, mproc->argv);
                        } else {
                            // Pipe setup
                            close(p1[0]);
                            close(p2[0]);
                            close(p2[1]);
                            dup2(p1[1], 1);
                            close(p1[1]);
                            // Error redirection setup
                            /*
                            if (lproc->err_file) {
                                if ((job->in_file && !strcmp(lproc->err_file, job->in_file)) || (job->out_file && !strcmp(lproc->err_file, job->out_file))) {
                                    fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE); }
                                int err_fd = open(lproc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                                if (err_fd < 0) {fprintf(stderr, RD_ERR); free_job(job); free(line); validate_input(NULL); exit(EXIT_FAILURE);}
                                int dup_status = dup2(err_fd, 2);
                                if (dup_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                                int close_status = close(err_fd);
                                if (close_status < 0) {fprintf(stderr, RD_ERR); exit(EXIT_FAILURE);}
                            }
                            */
                            // Execute command
                            execvp(lproc->cmd, lproc->argv);
                        }
                    }
                }
            // Non-piped process handling
            } else {
                // open err_file from the process and dup to stderr
                if (proc->err_file) {
                    if ((job->in_file && !strcmp(proc->err_file, job->in_file)) || (job->out_file && !strcmp(proc->err_file, job->out_file))) {
                        fprintf(stderr, RD_ERR);
                        free_job(job);
                        free(line);
                        validate_input(NULL);
                        exit(EXIT_FAILURE);
                    }

                    int err_fd = open(proc->err_file, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                    if (err_fd < 0) {
                        fprintf(stderr, RD_ERR);
                        free_job(job);
                        free(line);
                        validate_input(NULL);
                        exit(EXIT_FAILURE);
                    }

                    int dup_status = dup2(err_fd, 2);
                    if (dup_status < 0) {
                        fprintf(stderr, RD_ERR);
                        exit(EXIT_FAILURE);
                    }

                    int close_status = close(err_fd);
                    if (close_status < 0) {
                        fprintf(stderr, RD_ERR);
                        exit(EXIT_FAILURE);
                    }
                }

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
		    }
        } else { // Parent code
            // Close in file
            if (in_fd) {
                int close_status = close(in_fd);
                if (close_status < 0) {
                    perror(RD_ERR);
                    exit(EXIT_FAILURE);
                }
            }
            // Close out file
            if (out_fd) {
                int close_status = close(out_fd);
                if (close_status < 0) {
                    perror(RD_ERR);
                    exit(EXIT_FAILURE);
                }
            }
            sigprocmask(SIG_BLOCK, &mask_all, NULL); // block all signals
            // Add bg process to bg_list
            if (job->bg) {
                bgentry_t *bg_job = (bgentry_t*)malloc(sizeof(bgentry_t));
                bg_job->job = job;
                bg_job->pid = pid;
                bg_job->seconds = time(NULL);
                InsertInOrder(bg_list, bg_job);
                curr_bgprocs++;
            }
            sigprocmask(SIG_SETMASK, &mask_none, NULL); // unblock all signals
            // As the parent, wait for the foreground job to finish
			if (!job->bg) {
                wait_result = waitpid(pid, &exit_status, 0);
                if (wait_result < 0) {
			        printf(WAIT_ERR);
		            exit(EXIT_FAILURE);
	            }
                // Set exit_status of child
                else if (WIFEXITED(exit_status)) exit_status = WEXITSTATUS(exit_status);
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
