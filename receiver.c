#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "list.h"
#include "receiver.h"

static List* pReceiveList;
static pthread_t threadPID;
static pthread_mutex_t receiveListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t listNotEmptyCondVar = PTHREAD_COND_INITIALIZER;

void* receiveThread()
{
	while (1) {
		char message[MSG_MAX_LEN];
        int bytesReceived = recvfrom(socketDescriptor, message, MSG_MAX_LEN, 0, NULL, NULL);
        if (bytesReceived < 0) {
            General_print("Receive Thread Error: Failed to receive a message\n");
            continue;
        } 
       
        size_t messageSize;
        int terminateIndex;
        if (bytesReceived < MSG_MAX_LEN) {
            terminateIndex = bytesReceived;
            messageSize = bytesReceived + 1;
        } else {
            terminateIndex = MSG_MAX_LEN - 1;
            messageSize = MSG_MAX_LEN;
        }
        message[terminateIndex] = 0;
        char* pMessage = (char*)malloc(messageSize);
        memcpy(pMessage, message, messageSize);
        if (strcmp(pMessage, "!\n") == 0) {
            Receiver_addToReceiveList(pMessage);
            return NULL;
        }
        Receiver_addToReceiveList(pMessage);
	}
}

// Create a empty list and a UDP input thread that puts received message onto the newly created list
void Receiver_init()
{
    pReceiveList = List_create();
    if (pReceiveList == NULL) {
        General_print("Receive Thread Error: Failed to create the receive list\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threadPID, NULL, receiveThread, NULL) != 0) {
        General_print("Receive Thread Error: Failed to create the receive thread\n");
        exit(EXIT_FAILURE);
    }
}

// Add message to the receive list
void Receiver_addToReceiveList(char* message)
{
    pthread_mutex_lock(&receiveListMutex);
    {
        if (List_prepend(pReceiveList, message) == -1) {
            General_print("Receive Thread Error: Failed to add a received message to the receive list\n");
            free(message);
        } else if (List_count(pReceiveList) == 1) {
            pthread_cond_signal(&listNotEmptyCondVar);
        }
    }
    pthread_mutex_unlock(&receiveListMutex);
}

// Get the earliest received message from the receive list
char* Receiver_getFromReceiveList()
{
    char* message;
    pthread_mutex_lock(&receiveListMutex);
    {
        if (List_count(pReceiveList) == 0) {
            pthread_cond_wait(&listNotEmptyCondVar, &receiveListMutex);
        }
        message = (char*)List_trim(pReceiveList);
    }
    pthread_mutex_unlock(&receiveListMutex);
    return message;
}

// Cancel and wait for thread to finish, then cleanup memory
void Receiver_shutdown()
{
    pthread_cancel(threadPID);
    int result = pthread_join(threadPID, NULL);
    if (result != 0) {
        General_print("Receive Thread Error: Failed to cancel and join thread\n");
    }

    pthread_mutex_trylock(&receiveListMutex);
    pthread_mutex_unlock(&receiveListMutex);
    result = pthread_mutex_destroy(&receiveListMutex);
    if (result != 0) {
		General_print("Receive Thread Error: Failed to destroy the mutex\n");
	}

    result = pthread_cond_destroy(&listNotEmptyCondVar);
    if (result != 0) {
		General_print("Receive Thread Error: Failed to destroy the conditional variable\n");
	}

    List_free(pReceiveList, (*General_freeFunction));
}
