#include "hw2.h"
#include "helpers2.h"
#include "linkedlist.h"
#include "constants2.h"

// Part 1 Functions to implement
int getDate(char* str, Date* myDate) {
    // if either pointer is null, exit immediately
    if ((str == NULL) || (myDate == NULL)) return 0;
    
    char *str_ptr=str;
    short int digitCounter = 0;
    int month=0, day=0, year=0;

    // repeat process three times for month, day, year
    for (int i = 0; i < 3; i++) {
        // allocate space on the heap to store each term
        char *p = NULL;
        if (i < 2) p = (char*)malloc(3*sizeof(char));
        else p = (char*)malloc(5*sizeof(char));
        char *p_1 = p;
        // parse through str for each '/' partition
        while (*str_ptr != '/' && *str_ptr != '\0') {
            // check for non-numerical characters
            if (*str_ptr < 48 || *str_ptr > 57) {free(p); return 0;}
            // copy the character to the heap string
            *p_1 = *str_ptr;
            str_ptr++;
            p_1++;
            digitCounter++;
        }
        // increment once more to account for '/' character
        str_ptr++;
        *p_1 = '\0';
        // check if # of digits is valid
        if (i < 2 && digitCounter != 2) {free(p); return 0;}
        else if (i == 2 && digitCounter > 4) {free(p); return 0;}
 
        // convert each string to an integer
        if (i == 0) month = atoi(p);
        else if (i == 1) day = atoi(p);
        else year = atoi(p);

        // reset for next loop
        digitCounter = 0;
        free(p);
    }
    // check if values are valid
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    if (year < 0 || year > 2024) return 0;

    // assign values to Date
    myDate->month = month;
    myDate->day = day;
    myDate->year = year;

    return 1;
}

int cmpDate(const Date date1, const Date date2) {
    if (date1.year < date2.year) return -1;
    else if (date1.year > date2.year) return 1;
    else if (date1.month < date2.month) return -1;
    else if (date1.month > date2.month) return 1;
    else if (date1.day < date2.day) return -1;
    else if (date1.day > date2.day) return 1;
    else return 0;
}

// Part 2 Functions to implement
int strComparator(void* str1, void* str2) {
    if (str1 == NULL) {
        if (str2 == NULL) return 0;
        else return -1;
    } else if (str2 == NULL) return 1;
    else {
        char *ptr1 = (char*)str1, *ptr2 = (char*)str2;
        while (*ptr1 == *ptr2 && *ptr1 && *ptr2) {ptr1++; ptr2++;}
        if (*ptr1 < *ptr2) return -1;
        else if (*ptr1 > *ptr2) return 1;
        else return 0;
    }
}

void strPrinter(void* data, void* fp) {
    fprintf(fp, "%s", (char*)data);
}

void strDeleter(void* data) {
    if (data) {
        free(data);
        data = NULL;
    }
}

list_t* getGenres(char* str) {
    // chck for null and empty strings
    if (str == NULL) return NULL;
    else if (*str == '\0') return NULL;

    // create the result list
    list_t *result = CreateList(&strComparator, &strPrinter, &strDeleter);
    // initialize pointers for iterating through str
    char *curr_ptr = str, *begin_ptr = str;
    while (1) {
        // a genre is delimited by '|' (or '\n' or '\0' if it's the last one)
        if (*curr_ptr == '|' || *curr_ptr == '\n' || *curr_ptr == '\r' || *curr_ptr == '\0') {
            // allocate space on the heap for the genre characters + 1 (for \0)
            char *buffer = (char*)malloc(curr_ptr-begin_ptr+sizeof(char)), *buffer_ptr = buffer;
            // read each genre's character into the buffer
            while (begin_ptr < curr_ptr) {
                // convert all capital characters to lowercase
                if (*begin_ptr > 64 && *begin_ptr < 91) *buffer_ptr = *begin_ptr+32;
                else *buffer_ptr = *begin_ptr;
                begin_ptr++;
                buffer_ptr++;
            }
            // add null terminator to buffer
            *buffer_ptr = '\0';
            // insert the genre in order into the result
            InsertInOrder(result, (void*)buffer);
            if (*curr_ptr == '\n' || *curr_ptr == '\r' || *curr_ptr == '\0') return result;
            // increment the current genre pointers
            curr_ptr++;
            begin_ptr++;
            // if the next character is |, return null (empty genre found)
            if (*curr_ptr == '|') {
                DestroyList(&result);
                return NULL;
            }
        }
        // increment the current pointer
        else curr_ptr++;
    }
    // the function should never reach this point
    return result;
}

// Part 2 Generic Linked List functions
node_t* FindInList(list_t* list, void* token) {
    if (list == NULL || token == NULL) return NULL;
    node_t *ptr = list->head;
    while (ptr != NULL) {
        if (list->comparator(token, ptr->data) == 0) return ptr;
        ptr=ptr->next;
    }
    return NULL;
}

void DestroyList(list_t** list)  {
    // check to see if the list pointer is null
    if (list == NULL || *list == NULL) return;
    // initialize pointers for the list and the head node
    list_t *list_ptr = *list;
    node_t *node_ptr = list_ptr->head;
    // iterate through every node in the list
    while (node_ptr != NULL) {
        // set a temporary node pointer and move to the next node
        node_t *temp = node_ptr;
        node_ptr = node_ptr->next;
        // free the data and the node
        list_ptr->deleter(temp->data);
        free(temp);
        //list_ptr->deleter(temp);
    }
    // free the list
    free(*list);
    *list = NULL;
}

// Part 3 Functions to implement
void song_tShortPrinter(void* data, void *fp){
    if (data == NULL || fp == NULL) return;
    song_t *song = (song_t*)data;
    fprintf(fp, "%s: \"%s\", %s", song->artist, song->title, song->album);
}

void song_tVerbosePrinter(void* data, void *fp){
    if (data == NULL || fp == NULL) return;
    song_t *song = (song_t*)data;
    if (!song->liked) fprintf(fp, "%s: \"%s\", %s\n", song->artist, song->title, song->album);
    else fprintf(fp, "%s%s: \"%s\", %s%s\n", COLOR_START, song->artist, song->title, song->album, COLOR_RESET);
    fprintf(fp, "\tTimes Played: %d\n", song->freq);
    fprintf(fp, "\tLast Date Played: %02d/%02d/%04d\n", song->lastPlayed.month, song->lastPlayed.day, song->lastPlayed.year);
    fprintf(fp, "\tGenres: ");
    PrintLinkedList(song->genres, fp, ", ");
}

int song_tTitleComparator(void* lhs, void* rhs){
    if (!lhs || !rhs) return 0;
    song_t *lsong = (song_t*)lhs, *rsong = (song_t*)rhs;
    return strComparator(lsong->title, rsong->title);
}

int song_tFreqComparator(void* lhs, void* rhs){
    if (!lhs || !rhs) return 0; 
    song_t *lsong = (song_t*)lhs, *rsong = (song_t*)rhs;
    if (lsong->freq > rsong->freq) return -1;
    else if (lsong->freq < rsong->freq) return 1;
    else return 0;
}

int song_tLastPlayedComparator(void* lhs, void* rhs){
    if (!lhs || !rhs) return 0; 
    song_t *lsong = (song_t*)lhs, *rsong = (song_t*)rhs;
    int dateComp = cmpDate(lsong->lastPlayed, rsong->lastPlayed);
    if (dateComp) return -dateComp;
    else return 0;
}

void song_tDeleter(void* data){
    if (!data) return;
    song_t *song = (song_t*)data;
    strDeleter(song->artist);
    strDeleter(song->title);
    strDeleter(song->album);
    DestroyList(&song->genres);
    free(data);
}

song_t* createSong(char* line){
    if (!line || *line == '\0') return NULL;

    song_t *result = (song_t*)malloc(sizeof(song_t));
    char *curr_ptr = line, *begin_ptr = line;
    int columnCount = 0;
    while (1) {
        if (columnCount < 3 && (*curr_ptr > 127 || *curr_ptr < 0)) goto error;
        if (*curr_ptr == ',' || *curr_ptr == '\n' || *curr_ptr == '\r' || *curr_ptr == '\0') {
            char *buffer = (char*)malloc(curr_ptr - begin_ptr + sizeof(char)), *buffer_ptr = buffer;
            int bufferSize = 1;
            while (begin_ptr < curr_ptr) {
                *buffer_ptr = *begin_ptr;
                begin_ptr++;
                buffer_ptr++;
                bufferSize++;
            }
            *buffer_ptr = '\0';

            switch (columnCount) {
                case 0:     // title
                    result->title = (char*)malloc(bufferSize*sizeof(char));
                    string_copy(result->title, buffer);
                    free(buffer);
                    break;
                case 1:     // artist
                    result->artist =(char*)malloc(bufferSize*sizeof(char));
                    string_copy(result->artist, buffer);
                    free(buffer);
                    break;
                case 2:     // album
                    result->album =(char*)malloc(bufferSize*sizeof(char));
                    string_copy(result->album, buffer);
                    free(buffer);
                    break;
                case 3:     // liked
                    if (*buffer < '0' || *buffer > '1') {free(buffer); goto error;}
                    if (*buffer == '0') result->liked = 0;
                    else result->liked = 1;
                    free(buffer);
                    break;
                case 4: ;     // lastPlayed
                    Date d;
                    int dateStatus = getDate(buffer, &d);
                    if (!dateStatus) {free(buffer); goto error;}
                    result->lastPlayed = d;
                    free(buffer);
                    break;
                case 5: ;     // freq
                    int freqInt = atoi(buffer);
                    if (freqInt < 0) {free(buffer); goto error;}
                    result->freq = freqInt;
                    free(buffer);
                    break;
                case 6: ;     // genres
                    list_t *genreList = getGenres(buffer);
                    if (!genreList) {free(buffer); goto error;}
                    result->genres = genreList;
                    free(buffer);
                    break;
            }
            if (*curr_ptr == '\n' || *curr_ptr == '\r' || *curr_ptr == '\0') return result;
            columnCount++;
            curr_ptr++;
            begin_ptr++;
            if (*curr_ptr == ',') goto error;
        } else {
            curr_ptr++;
        }
    }
    error: ;
        if (columnCount > 0) free(result->artist);
        if (columnCount > 1) free(result->title);
        if (columnCount > 2) free(result->album);
        free(result);
        return NULL;
}

int printNSongs(list_t* list, FILE* fp, int NUM, int liked_flag){
    if (!list || !fp || NUM < 0) return -1;

    node_t *curr = list->head;
    int currCount = 0;
    if (!NUM) NUM = list->length;
    while (currCount < NUM && currCount < list->length && curr) {
        if (!liked_flag) {
            list->printer(curr->data, fp);
            fprintf(fp, "\n");
        } else if (liked_flag && ((song_t*)curr->data)->liked+48 == '1') {
            list->printer(curr->data, fp);
            fprintf(fp, "\n");
        }
        curr = curr->next;
        currCount++;
    }
    return currCount;
}



