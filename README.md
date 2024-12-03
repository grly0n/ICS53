This repository stores all class projects from ICS53: Principles in System Design

## HW5: ZotPoll

A demonstration of Networking and Concurrency. Client processes connect to a running server process via a socket connection. The server process spawns and manages threads that handle clients, terminating upon completion.
Clients can make requests to log in, list information about all polls, vote for a poll, list statistics about polls the client has voted for, and log out.
The server employs mutexes on shared variables to ensure that safe concurrency is maintained for the entire duration of runtime.

The clients and server communicate via a custom PetrV protocol, consisting of a header containing the type and length of message sent, followed by the message body if applicable.
The server includes error handling if a client submits a bad request, as well as clean up of dynamically allocated memory and proper termination of all client threads upon receiving SIGINT.
