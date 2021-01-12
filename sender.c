#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "input.h"
#include "sender.h"

static pthread_t threadPID;
static struct sockaddr_in sinRemote;

void* sendThread()
{
	while (1) {
		char* pMessage = Input_getFromSendList();
		int bytesSent = sendto(socketDescriptor, pMessage, strlen(pMessage), 0, (struct sockaddr *)&sinRemote, sizeof(sinRemote));
        if (bytesSent < 0) {
            General_print("Send Thread Error: Failed to send a message\n");
        }

        if (strcmp(pMessage, "!\n") == 0) {
            free(pMessage);
            General_terminate();
            return NULL;
        }
        free(pMessage);
	}
}

// Create a thread that sends message to the remote user
void Sender_init(char* machineName, char* port)
{
    if (pthread_create(&threadPID, NULL, sendThread, NULL) != 0) {
        General_print("Send Thread Error: Failed to create the send thread\n");
        exit(EXIT_FAILURE);
    }

    memset(&sinRemote, 0, sizeof(sinRemote));
    sinRemote.sin_family = AF_INET;
    sinRemote.sin_port = htons(atoi(port));

    // IPv4 address
    struct hostent *host_entry = gethostbyname(machineName);
    if (host_entry == NULL) {
        General_print("Send Thread Error: Failed to get host by name\n");
        exit(EXIT_FAILURE);
    }

    const char* ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    if (ip == NULL) {
        General_print("Send Thread Error: Failed to convert the host address to a string in the Internet standard dot notation\n");
        exit(EXIT_FAILURE);
    }

    int result = inet_pton(AF_INET, ip, &sinRemote.sin_addr.s_addr);
    if (result != 1) {
        General_print("Send Thread Error: Failed to store binary form of host's IPv4 address\n");
        exit(EXIT_FAILURE);
    }
}

// Cancel and wait for thread to finish
void Sender_shutdown()
{    
    pthread_cancel(threadPID);
    int result = pthread_join(threadPID, NULL);
    if (result != 0) {
        General_print("Send Thread Error: Failed to cancel and join thread\n");
    }
}
