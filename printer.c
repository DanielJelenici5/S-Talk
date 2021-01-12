#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "printer.h"
#include "receiver.h"

static pthread_t threadPID;

void* printThread()
{
	while (1) {
		char* pMessage = Receiver_getFromReceiveList();
        if (strcmp(pMessage, "!\n") == 0) {
            free(pMessage);
            General_terminate();
            return NULL;
        }

        General_printAndCheck(pMessage, "Print Thread Error: Failed to display received message\n");
        free(pMessage);
	}
}

// Create a thread that prints received message
void Printer_init()
{
    if (pthread_create(&threadPID, NULL, printThread, NULL) != 0) {
        General_print("Print Thread Error: Failed to create the receive thread\n");
        exit(EXIT_FAILURE);
    }
}

// Cancel and wait for thread to finish
void Printer_shutdown()
{
    pthread_cancel(threadPID);
    int result = pthread_join(threadPID, NULL);
    if (result != 0) {
        General_print("Print Thread Error: Failed to cancel and join thread\n");
    }
}
