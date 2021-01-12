#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "general.h"

int socketDescriptor;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t programTerminatedCondVar = PTHREAD_COND_INITIALIZER;

//Initialize Socket
void General_socketInit(char* port)
{
	// Address
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(atoi(port));
	// Create the socket for UDP
	socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        General_print("General.c: Failed to create the socket descriptor\n");
        exit(EXIT_FAILURE);
    }
	// Bind the socket to the specified port
	int result = bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
    if (result == -1) {
        General_print("General.c: Failed to bind the socket to the specified port\n");
        exit(EXIT_FAILURE);
    }
}

// Function pointer for List_free()
void freeItem(void* item) 
{
	free(item);
}
void (*freeFunction)(void*) = &freeItem;

// Display message
void General_print(char* message)
{
	write(fileno(stdout), message, strlen(message));
}

// Display message, and display error message on failure
void General_printAndCheck(char* message, char* errorMessage)
{
	if (write(fileno(stdout), message, strlen(message)) < 0) {
		write(fileno(stdout), errorMessage, strlen(errorMessage));
	}
}

// Wait until the program terminates
void General_waitForTermination()
{
	pthread_mutex_lock(&mutex);
    {
        pthread_cond_wait(&programTerminatedCondVar, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

// Signal to terminate the program
void General_terminate()
{
	pthread_mutex_lock(&mutex);
    {
        pthread_cond_signal(&programTerminatedCondVar);
    }
    pthread_mutex_unlock(&mutex);
	General_print("\nPROGRAM TERMINATED\n");
}

// Cleanup used resources
void General_cleanup()
{
	int result;
    result = close(socketDescriptor);
	if (result != 0) {
		General_print("General.c: Failed to close the socket\n");
	}

	result = pthread_mutex_destroy(&mutex);
	if (result != 0) {
		General_print("General.c: Failed to destroy the mutex\n");
	}

    result = pthread_cond_destroy(&programTerminatedCondVar);
	if (result != 0) {
		General_print("General.c: Failed to destroy the conditional variable\n");
	}
}
