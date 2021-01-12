#ifndef _GENERAL_H_
#define _GENERAL_H_

#define MSG_MAX_LEN 512

//Initialize Socket
int socketDescriptor;
void General_socketInit(char* port);

// Function pointer for List_free()
void (*General_freeFunction)(void*);

// Display message
void General_print(char* message);

// Display message, and display error message on failure
void General_printAndCheck(char* message, char* errorMessage);

// Wait until the program terminates
void General_waitForTermination();

// Signal to terminate the program
void General_terminate();

// Cleanup used resources
void General_cleanup();

#endif
