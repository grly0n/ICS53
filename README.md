This repository stores all class projects from ICS53: Principles in System Design

## HW1: 53wc
Practice with learning core C syntax and functionality, with an emphasis on positional command line arguments, standard library functions, and file I/O.

The program is a modification of the UNIX `wc` command, reading from `stdin` and printing to `stdout`. Input can be counted by line count, symbol (non-alphanumeric character) occurence,
number occurence, occurences of a given amount of consecutive spaces, and lines that are blank, contain only whitespace characters, or include leading and/or trailing whitespace.

With the consecutive spaces option, all occurences are "compressed" into a single tab character.

With the whitespace option, all leading and trailing whitespace is removed from each line, and all blank lines are removed before printing to the output. The input remains unchanged.

## HW2: 53Music
Further practice with learning core C syntax and functionality, with an emphasis on pointers, array manipulation, strings, and structures. For this assignment, no array indexing or string libraries were used.
Everything was done using pointer manipulation.

The program parses a CSV file containing information about songs and prints songs based on given criteria. Options for searching include songs last played after a certain date, songs that include a certain genre
in its list of genres, and songs that contain a keyword in the album, artist, or title fields. Other output options are only returning a certain number of results, only returning liked songs, printing with a
verbose format, and specifiying input and output files.


## HW3: Simple Shell
A demonstration of low-level UNIX system calls related to processes, file access, and interprocess communication. The shell forks a child process for every command given in a user argument, with piped
commands forking a child per job. Processes can be initialized to run in the background and cleaned up upon the user entering the next command, or pulled to run in the foreground with the built-in `fg`.
Another built-in `bglist` lists all currently running background processes, with a maximum given as an argument to the shell when first run.

Upon termination of the shell, all background processes are killed before the shell terminates. The shell prints the current date and time upon receiving `SIGUSR2`.

The shell supports input, output, and error redirection to a file, checking for invalid combinations (e.g. redirecting input and output from and to the same file).
Jobs can be piped up to three processes long.


## HW4: Explicit Memory Allocator
A demonstration of Dynamic Memory Allocation. Code consists of an implementation of an ***explicit free list*** memory allocator with ***next-fit placement, address-ordered, forward coalescing only,*** and ***block splitting without splinters.***
Free blocks are arranged in a doubly linked list which is searched for an adequately sized space upon `ics_malloc()` and updated with coalescing upon every call to `ics_free()`.
`ics_realloc()` reallocates a given block to a block of a new size, copying all data from the old to new block and truncating if the size of the new is less than the old.
Free blocks contain in the payload space the addresses of the next and previous free blocks, with a `NULL` next being associated with the end of the list and a `NULL` previous associated with the head.
Two global variables in the allocator track the current position of the freelist head and next block to begin searching at.

Words are 16-byte aligned, with all blocks including an 8-byte header and footer, and a minimum of 16 bytes of payload.

A maximum of five 4096-byte pages can be requested by the allocator before returning with an error.

## HW5: ZotPoll

A demonstration of Networking and Concurrency. Client processes connect to a running server process via a socket connection. The server process spawns and manages threads that handle clients, terminating upon completion.
Clients can make requests to log in, list information about all polls, vote for a poll, list statistics about polls the client has voted for, and log out.
The server employs mutexes on shared variables to ensure that safe concurrency is maintained for the entire duration of runtime.

The clients and server communicate via a custom PetrV protocol, consisting of a header containing the type and length of message sent, followed by the message body if applicable.
The server includes error handling if a client submits a bad request, as well as clean up of dynamically allocated memory and proper termination of all client threads upon receiving `SIGINT`.
