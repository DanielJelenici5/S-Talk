#ifndef _SENDER_H_
#define _SENDER_H_

// Start background send thread
void Sender_init(char* port, char* machineName);

// Stop background send thread
void Sender_shutdown();

#endif
