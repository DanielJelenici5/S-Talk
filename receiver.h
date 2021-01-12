#ifndef _RECEIVER_H_
#define _RECEIVER_H_

// Start background receive thread
void Receiver_init();

// Add message to the receive list
void Receiver_addToReceiveList(char* receivedMessage);

// Retrieve the earliest message from the receive list
char* Receiver_getFromReceiveList();

// Stop background receive thread and cleanup
void Receiver_shutdown();

#endif
