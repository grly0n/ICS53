#include "hw2.h"
#include "linkedlist.h"
#include "helpers2.h"
#include <string.h> // Not allowed in your HW implemention!!

int main(int argc, char *argv[]) {

    char* str1 = "Pop";  	   // Static string
    char str2[] = "Classical"; 	   // Array without size, edittable
    char str3[12] = "Hip Hop";   // Array with size, uninitialized bytes set to 0 
    char str4[4] = "Hip Hop";    // Array with size, truncated initialization (no null terminator!) DON'T USE THIS AS A STRING!!!  Will cause a compilation warning

    char* str5 = calloc(5,1);    // 5 bytes of space initialized to 0
    strncpy(str5,str4,4);         // String.h functions are not allowed in your HW implemention!! Create your own versions with pointers

    printf("\n******Compare Tests******\n");    
    printf("%s vs %s, strComparator returns %d\n", str1, str2, strComparator(str1, str2));
    printf("%s vs %s, strComparator returns %d\n", str2, str1, strComparator(str2, str1));
    printf("%s vs %s, strComparator returns %d\n", str2, str2, strComparator(str2, str2));
    printf("%s vs %s, strComparator returns %d\n", str4, str5, strComparator(str3, str5));



    printf("\n******Output Test******\n");
    printf("Printing str1 to stdout: ");
    strPrinter(str1, stdout);
    printf("\nPrinting str2 to stdout.txt");
    FILE *f = fopen("tests/stdout.txt", "w");
    strPrinter(str2, f);
    fclose(f);
    f = fopen("tests/stdout.txt", "r");
    char buf[10];
    fread(buf, sizeof(char), 10, f);
    printf("\nContents of stdout.txt: %s\n", buf);
    fclose(f);
    remove("tests/stdout.txt");



printf("\n\n******Deleter Test******\n");
    strDeleter(str5);   
    if(str5 == NULL)
        printf("How did you do that?!?!");
    else
        str5 = NULL;

    //What happens if you try to delete str1 or str2? Why? Does it impact anything?
    //strDeleter(str1);
    //strDeleter(str2);
    
    printf("\n\n******getGenres Tests******\n");
    list_t* genre1 = getGenres("Electronic|Experimental|Rock|Neo-psychedelia\n");
    //printf("\n***Expected: country->pop->rock***\n");
    //PrintLinkedList(genre1, stdout, "->"); 
    printf("\n");
    
    list_t* genre2 = getGenres("neo-psychedelia|electronic|classic rock|experimental\n");
    printf("\n***Expected: classic rock->electronic->experimental->neo-psychedelia***\n");
    PrintLinkedList(genre2, stdout, "->"); 
    printf("\n");

    printf("\n\n***Error creating list***\n");
    list_t* genre3 = getGenres("techno||Classical\n");
    if(genre3)
        printf("OOPS! No list should be created in this error case\n");
    else
        printf("YEAH! No list created!\n");



    printf("\n\n******FindInList Tests******\n");
    printf("\n***Search for rock in List country->pop->rock***\n");
    node_t* item = FindInList(genre1, "rock"); 
    if(!item)
        printf("OOPS! It should be there\n");
    else
        printf("FOUND!\n");

   printf("\n***Search for country in List country->pop->rock***\n");
    node_t* item2 = FindInList(genre1, "country"); 
    if(!item2)
        printf("OOPS! It should be there\n");
    else
        printf("FOUND!\n");


    printf("\n***Search for classic ROCK in List classic rock->electronic->experimental->neo-psychedelia***\n");
    node_t* item3 = FindInList(genre2, "classic ROCK"); 
    if(!item3)
        printf("YEAH! classic ROCK not was found in list\n");
    else
        printf("OOPS! How was this found?\n");

    printf("\n***Search for pop in List classic rock->electronic->experimental->neo-psychedelia***\n");
    node_t* item4 = FindInList(genre2, "pop"); 
    if(!item4)
        printf("YEAH! pop was not found in list\n");
    else
        printf("OOPS! How was this found?\n");

    printf("\n\n******DestroyList Tests******\n");
    printf("\n***Delete List 1***\n");
    DestroyList(&genre1);

    if(genre1)
        printf("OOPS! Don't forget to set the pointer to NULL after free0-ng the memory\n");
    else
        printf("genre1 list deleted\n");

    //This should print nothing!
    PrintLinkedList(genre1, stderr, "");

    printf("\n***Delete List 2***\n");
    DestroyList(&genre2);

    if(genre2)
        printf("OOPS! Don't forget to set the pointer to NULL after free0-ng the memory\n");
    else
        printf("genre2 list deleted\n");

    //This should print nothing!
    PrintLinkedList(genre2, stderr, "");

    // Check Valgrind! All memory should be free!
   
    //list_t *list = CreateList(&song_tTitleComparator, &song_tShortPrinter, &song_tDeleter);  
    //song_t *song1 = createSong("Mr. Know-it-all,Young the Giant,Home of the Strange,1,05/24/2001,2000,rock\n");
    //song_t *song2 = createSong("No Control,Willow,<COPINGMECHANISM>,0,04/09/2022,20,ROck\n");
    //song_t *song3 = createSong("Buddy Holly,Weezer,Weezer,1,04/12/2002,2002,ELECTRONIC|ROCK|experimental|neo-psychedelia\n");
    //song_t *song2 = createSong("Pink Pony Club,Chappell Roan,The Rise and Fall of a Midwest Princess,1,08/23/2024,1234567,Pop|synth-pop\n");
    //song_t *song3 = createSong("In The Flesh,Pink Floyd,The Wall,0,10/10/2010,1,Rock|Racist Rock\n");

    //InsertInOrder(list, song1);
    //InsertInOrder(list, song2);
    //InsertInOrder(list, song3);

    //char *genre = (char*)malloc(5*sizeof(char));
    //string_copy_lowercase(genre, "experimental");

    //if (FindInList(song1->genres, genre)) song_tShortPrinter(song1, stdout);
    //printf("\n");
    //if (FindInList(song2->genres, genre)) song_tShortPrinter(song2, stdout);
    //printf("\n");
    //if (FindInList(song3->genres, genre)) song_tShortPrinter(song3, stdout);
    //printNSongs(list, stdout, 0, 1);
    //DestroyList(&list);

    char *string1 = "pink pony club", *string2 = "pi";
    printf("%d\n", containsSubstring(string1, string2));

    printf("\n******END OF TEST******\n");
    return 0;
}
