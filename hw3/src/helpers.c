// Your helper functions need to be here.
#include "helpers.h"
#include "linkedlist.h"
#include "icssh.h"

int invalid_argc_or_length(job_info *job) {
    proc_info *curr_proc = job->procs;
    while(curr_proc) {
        if (curr_proc->argc > 32) return 1;
        else if (curr_proc->argc > 1) {
            int i = 1;
            for(i; i < curr_proc->argc; i++) {
                if (strlen(curr_proc->argv[i]) > 63) return 1;
            }
        }
        curr_proc = curr_proc->next_proc;
    }
    return 0;
}

// compares time_t seconds of two bgentry_t instances (highest -> lowest)
int list_comparator(const void *lhs, const void *rhs) {
    bgentry_t *lhs_job = (bgentry_t*)lhs, *rhs_job = (bgentry_t*)rhs;
    if (lhs_job->seconds < rhs_job->seconds) return 1;
    else if (rhs_job->seconds < lhs_job->seconds) return -1;
    else return 0;
}

// prints a bgentry_t instance's data to fp
void list_printer(void *job, void* fp) {
    bgentry_t *bg_job = (bgentry_t*)job;
    FILE *output = (FILE*)fp;
    fprintf(output, "%ld\t%d\t%s", bg_job->seconds, bg_job->pid, bg_job->job->line);
}

void list_deleter(void *job) {
    bgentry_t *bg_job = (bgentry_t*)job;
    free_job(bg_job->job);
    free(bg_job);
}

// removes a background job given its pid from the linked list of background jobs
int removePidFromList(list_t *list, pid_t pid, int bg) {
    node_t *curr_node = list->head;
    int index = 0;
    while (curr_node) {
        bgentry_t *curr_data = (bgentry_t*)curr_node->data;
        if (curr_data->pid == pid) {
            if (bg) fprintf(stdout, BG_TERM, curr_data->pid, curr_data->job->line);
            list->deleter(curr_data);
            RemoveByIndex(list, index); 
            return 1;
        } 
        curr_node = curr_node->next;
        index++;
    }
    return 0;
}

node_t *listContainsPid(list_t *list, pid_t pid) {
    node_t *curr_node = list->head;
    while (curr_node) {
        bgentry_t *curr_data = (bgentry_t*)curr_node->data;
        if (curr_data->pid == pid) return curr_node;
        curr_node = curr_node->next;
    }
    return NULL;
}
