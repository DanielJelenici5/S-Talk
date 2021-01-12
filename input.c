#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "general.h"
#include "input.h"
#include "list.h"

static List* pSendList;
static pthread_t threadPID;
static pthread_mutex_t sendListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t listNotEmptyCondVar = PTHREAD_COND_INITIALIZER;

void* inputThread()
{	
    while (1) {
        char message[MSG_MAX_LEN];

        int bytesRead = 0;
        for (int i = 0; i < MSG_MAX_LEN; i++) {
            bytesRead += read(fileno(stdin), &message[i], 1);
            if (message[i] == '\n') {
                break;
            }
        }
        if (bytesRead < 0) {
            General_print("Input Thread Error: Failed to read the message\n");
            continue;
        }

        size_t messageSize;
        int terminateIndex;
        if (bytesRead < MSG_MAX_LEN) {
            terminateIndex = bytesRead;
            messageSize = bytesRead + 1;
        } else {
            terminateIndex = MSG_MAX_LEN - 1;
            messageSize = MSG_MAX_LEN;
        }
        message[terminateIndex] = 0;
        char* pMessage = (char*)malloc(messageSize);
        memcpy(pMessage, message, messageSize);
        if (strcmp(pMessage, "!\n") == 0) {
            Input_addToSendList(pMessage);
            return NULL;
        }
        Input_addToSendList(pMessage);
    }
}

// Create a empty list and a thread that adds user input to the newly created list
void Input_init()
{
    pSendList = List_create();
    if (pSendList == NULL) {
        General_print("Input Thread Error: Failed to create the send list\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&threadPID, NULL, inputThread, NULL) != 0) {
        General_print("Input Thread Error: Failed to create the input thread\n");
        exit(EXIT_FAILURE);
    }
}

// Add input to the send list
void Input_addToSendList(char* message)
{
    pthread_mutex_lock(&sendListMutex);
    {
        if (List_prepend(pSendList, message) == -1) {
            General_print("Input Thread Error: Failed to add the input to the send list\n");
            free(message);
        } else if (List_count(pSendList) == 1) {
            pthread_cond_signal(&listNotEmptyCondVar);
        }
    }
    pthread_mutex_unlock(&sendListMutex);
}

// Get the earliest message from the send list
char* Input_getFromSendList()
{
    char* message;
    pthread_mutex_lock(&sendListMutex);
    {
        if (List_count(pSendList) == 0) {
            pthread_cond_wait(&listNotEmptyCondVar, &sendListMutex);
        }
        message = (char*)List_trim(pSendList);  
    }
    pthread_mutex_unlock(&sendListMutex);
    return message;
}

// Cancel and wait for thread to finish, then cleanup memory
void Input_shutdown()
{
    pthread_cancel(threadPID);
    int result = pthread_join(threadPID, NULL);
    if (result != 0) {
        General_print("Input Thread Error: Failed to cancel and join thread\n");
    }

    pthread_mutex_trylock(&sendListMutex);
    pthread_mutex_unlock(&sendListMutex);
    result = pthread_mutex_destroy(&sendListMutex);
    if (result != 0) {
        General_print("Input Thread Error: Failed to destroy the mutex\n");
    }
    
    result = pthread_cond_destroy(&listNotEmptyCondVar);
    if (result != 0) {
        General_print("Input Thread Error: Failed to destroy the conditional variable\n");
    }

    List_free(pSendList, (*General_freeFunction));
}
