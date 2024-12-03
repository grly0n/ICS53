#include "constants2.h"
#include "linkedlist.h"
#include "helpers2.h"
#include "hw2.h"

int main(int argc, char* argv[]) {
	int D_flag = 0, G_flag = 0, K_flag = 0, NUM_arg = 0, liked_flag = 0, verbose_flag = 0;
    char *arg_genre = NULL,  *arg_keyword = NULL;  
    Date arg_date;
    char *OUTFILE = NULL, *INFILE = NULL;

    // Use basic getopt to parse flags and respective arguments
    int c;
    while ((c = getopt(argc, argv, "HD:G:K:n:lvo:i:" )) >= 0) {
        switch (c) {
            case 'H':
				fprintf(stdout, USAGE_MSG);
				return EXIT_SUCCESS;
            case 'D':
        	    D_flag = 1;
				if(!getDate(optarg,&arg_date))
                    return EXIT_FAILURE;
                break;
            case 'G':
				G_flag = 1;
				arg_genre = optarg;
                break;
            case 'K':
				K_flag = 1;
				arg_keyword = optarg;
                break;
            case 'n':
				NUM_arg = atoi(optarg);     // basic conversion only. Add arg checking.
                if (NUM_arg < 0 || !NUM_arg) {
                    fprintf(stderr, USAGE_MSG);
                    return 1;
                }
                break;
            case 'l':
				liked_flag = 1;
                break;
            case 'v':
				verbose_flag = 1;
                break;
            case 'o':
				OUTFILE = optarg;
                break;
            case 'i':
				INFILE = optarg;
                break;
            default:
                fprintf(stderr, USAGE_MSG);
                return EXIT_FAILURE;
        }
    }

    // validate a required option was specified - Does not check for more than 1!
    if ( ! (D_flag | G_flag | K_flag) )
    {
        fprintf(stderr, "ERROR: Reuqired option was not specified.\n\n" USAGE_MSG);
        return EXIT_FAILURE;
    }

    // initialize variables for reading from input
    char* line = NULL;
    size_t size = 0;
    list_t *list = NULL;
    
    // INSERT YOUR IMPLEMENTATION HERE
    // getopts only stored the arguments and performed basic checks. More error checking is still needed!!!!
    if (D_flag) {
        if (G_flag || K_flag) {fprintf(stderr, USAGE_MSG); return 1;}

        if (verbose_flag) list = CreateList(&song_tLastPlayedComparator, &song_tVerbosePrinter, &song_tDeleter);
        else list = CreateList(&song_tLastPlayedComparator, &song_tShortPrinter, &song_tDeleter);

        if (!INFILE) {
            while (getline(&line, &size, stdin) != -1) {
                song_t *song = createSong(line);
                if (!song) continue;
                if (cmpDate(arg_date, song->lastPlayed) <= 0)
                    if ((liked_flag && song->liked) || (!liked_flag))   
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);
            }
        }
        else {
            FILE* infile = fopen(INFILE, "r");
            if (!infile) goto fileError;
            while (getline(&line, &size, infile) != -1) {
                song_t *song = createSong(line);               
                if (!song) continue;
                if (cmpDate(arg_date, song->lastPlayed) <= 0)
                    if ((liked_flag && song->liked) || (!liked_flag))
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);
            }
            int closeStatus = fclose(infile);
            if (closeStatus) goto fileError;
        }

        if (!OUTFILE) {
            printNSongs(list, stdout, NUM_arg, liked_flag);
        } else {
            FILE* outfile = fopen(OUTFILE, "w");
            if (!outfile) goto fileError;
            printNSongs(list, outfile, NUM_arg, liked_flag);
            int closeStatus = fclose(outfile);
            if (closeStatus) goto fileError;
        }
        goto successExit;

    } else if (G_flag) {
        if (D_flag || K_flag) {fprintf(stderr, USAGE_MSG); return 1;}
        
        // initialize list_t
        if (verbose_flag) list = CreateList(&song_tTitleComparator, &song_tVerbosePrinter, &song_tDeleter);
        else list = CreateList(&song_tTitleComparator, &song_tShortPrinter, &song_tDeleter);

        char *genre_lowercase = (char*)malloc(string_length(arg_genre)*sizeof(char));
        string_copy_lowercase(genre_lowercase, arg_genre);

        // read from input
        if (!INFILE) {
            while (getline(&line, &size, stdin) != -1) {
                song_t *song = createSong(line);
                if (!song) continue;
                if (FindInList(song->genres, genre_lowercase))
                    if ((liked_flag && song->liked) || (!liked_flag))
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);
            }
        } 
        else {
            FILE* infile = fopen(INFILE, "r");
            if (!infile) {
                free(genre_lowercase);
                goto fileError;
            }
            while (getline(&line, &size, infile) != -1) {
                song_t *song = createSong(line);
                if (!song) continue;
                if (FindInList(song->genres, genre_lowercase))
                    if ((liked_flag && song->liked) || (!liked_flag))
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);
            }
            int closeStatus = fclose(infile);
            if (closeStatus) {
                free(genre_lowercase);
                goto fileError;
            }
        }

        free(genre_lowercase);

        // print to output
        if (!OUTFILE) {
            printNSongs(list, stdout, NUM_arg, liked_flag);
        } else {
            FILE* outfile = fopen(OUTFILE, "w");
            if (!outfile) goto fileError;
            printNSongs(list, outfile, NUM_arg, liked_flag);
            int closeStatus = fclose(outfile);
            if (closeStatus) goto fileError;
        }
        goto successExit;

    } else if (K_flag) {
        if (D_flag || G_flag) {fprintf(stderr, USAGE_MSG); return 1;}
        
        // initialize list_t
        if (verbose_flag) list = CreateList(&song_tFreqComparator, &song_tVerbosePrinter, &song_tDeleter);
        else list = CreateList(&song_tFreqComparator, &song_tShortPrinter, &song_tDeleter);

        // initialize space for lowercase keyword and words
        char *keyword_lowercase = (char*)malloc(string_length(arg_keyword));
        string_copy_lowercase(keyword_lowercase, arg_keyword);
        char *artist_l = NULL, *title_l = NULL, *album_l = NULL;

        // read from input
        if (!INFILE) {
            while (getline(&line, &size, stdin) != -1) {
                song_t *song = createSong(line);
                if (!song) continue;
                artist_l = (char*)malloc(string_length(song->artist)*sizeof(char));
                title_l = (char*)malloc(string_length(song->title)*sizeof(char));
                album_l = (char*)malloc(string_length(song->album)*sizeof(char));
                string_copy_lowercase(artist_l, song->artist);
                string_copy_lowercase(title_l, song->title); 
                string_copy_lowercase(album_l, song->album);
                if (containsSubstring(artist_l, keyword_lowercase) || containsSubstring(title_l, keyword_lowercase) || containsSubstring(album_l, keyword_lowercase))
                    if ((liked_flag && song->liked) || (!liked_flag))
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);

                free(artist_l);
                free(title_l);
                free(album_l);
            }
        } 
        else {
            FILE* infile = fopen(INFILE, "r");
            if (!infile) {
                free(keyword_lowercase);
                goto fileError;
            }
            while (getline(&line, &size, infile) != -1) {
                song_t *song = createSong(line);               
                if (!song) continue;
                artist_l = (char*)malloc(string_length(song->artist)*sizeof(char));
                title_l = (char*)malloc(string_length(song->title)*sizeof(char));
                album_l = (char*)malloc(string_length(song->album)*sizeof(char));
                string_copy_lowercase(artist_l, song->artist);
                string_copy_lowercase(title_l, song->title); 
                string_copy_lowercase(album_l, song->album);
                if (containsSubstring(artist_l, keyword_lowercase) || containsSubstring(title_l, keyword_lowercase) || containsSubstring(album_l, keyword_lowercase))  
                    if ((liked_flag && song->liked) || (!liked_flag))
                        InsertInOrder(list, song);
                    else song_tDeleter(song);
                else song_tDeleter(song);

                free(artist_l);
                free(title_l);
                free(album_l);
            }
            int closeStatus = fclose(infile);
            if (closeStatus) {
                free(keyword_lowercase);
                goto fileError;
            }
        }

        free(keyword_lowercase);

        // print to output
        if (!OUTFILE) {
            printNSongs(list, stdout, NUM_arg, liked_flag);
        } else {
            FILE* outfile = fopen(OUTFILE, "w");
            if (!outfile) goto fileError;
            printNSongs(list, outfile, NUM_arg, liked_flag);
            int closeStatus = fclose(outfile);
            if (closeStatus) goto fileError;
        }
        goto successExit;

    }

    fileError:
        free(line);
        DestroyList(&list);
        return 2;

    successExit:
        DestroyList(&list);
        free(line);
        return 0;
}
