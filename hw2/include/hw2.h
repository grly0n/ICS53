// DO NOT MODIFY THIS FILE
// Any additions should be placed in helpers2.h

#ifndef HW_2_H
#define HW_2_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "linkedlist.h"

typedef struct date_t
{
	unsigned short int month;
	unsigned short int day;
	unsigned short int year;

} Date;

typedef struct
{
	char *artist;        // artist name  
	char *title;         // song Title
	char *album;         // title of the album
	char liked;          // "liked" song flag. 0x1 for liked, 0x0 for not liked
	Date lastPlayed;     // Date struct for last time song was played
	unsigned int freq;       // number of times a song has been played
	list_t* genres;      // Linked List of genre(s) 
} song_t;

// Part 1 Functions to implement
int getDate(char* str, Date* myDate);
int cmpDate(Date date1, Date date2);

// Part 2 Functions to implement
int strComparator(void* str1, void* str2);
void strPrinter(void* data, void* fp);
void strDeleter(void* data);
list_t* getGenres(char* str);

node_t* FindInList(list_t* list, void* token);
void DestroyList(list_t** list);

// Part 3 Functions to implement
void song_tShortPrinter(void* data, void *fp);
void song_tVerbosePrinter(void* data, void *fp);
int song_tTitleComparator(void* lhs, void* rhs);
int song_tFreqComparator(void* lhs, void* rhs);
int song_tLastPlayedComparator(void* lhs, void* rhs);
void song_tDeleter(void* data);
song_t* createSong(char* line);
int printNSongs(list_t* list, FILE* fp, int NUM, int liked_flag);

#endif
