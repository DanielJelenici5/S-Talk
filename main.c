#include <stdio.h>
#include "general.h"
#include "input.h"
#include "sender.h"
#include "receiver.h"
#include "printer.h"

int main(int argc, char** args)
{
    char* port = args[1];
    char* remoteMachineName = args[2];
    char* remotePort = args[3];
    General_print("WELCOME TO S-TALK\n");
    General_print("===============================================================\n");
    General_print("Your port number: ");
    General_print(port);
    General_print("\nRemote user machine name: ");
    General_print(remoteMachineName);
    General_print("\nRemote user port number: ");
    General_print(remotePort);
    General_print("\n\n");

    General_socketInit(port);
    Input_init();
    Sender_init(remoteMachineName, remotePort);
    Receiver_init();
    Printer_init();

    General_waitForTermination();

    Printer_shutdown();
    Receiver_shutdown();
    Sender_shutdown();
    Input_shutdown();
    General_cleanup();

    General_print("EXITING S-TALK\n");
    return 0;
}
