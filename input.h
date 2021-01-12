#ifndef _INPUT_H_
#define _INPUT_H_

// Start background input thread
void Input_init();

// Add input to the send list
void Input_addToSendList(char* message);

// Get the earliest message from the send list
char* Input_getFromSendList();

// Stop background input thread and cleanup
void Input_shutdown();

#endif
