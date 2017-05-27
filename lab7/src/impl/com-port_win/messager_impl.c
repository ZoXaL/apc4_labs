#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "messager.h"
#include "messager_settings.h"

extern unsigned short isFirst;

HANDLE firstSemaphore;
HANDLE secondSemaphore;

HANDLE listenerThreadHandle;
HANDLE secondSemaphore;

HANDLE firstAcceptMessageEvent;
HANDLE secondAcceptMessageEvent;

HANDLE messagePort;
int deathFlag = 0;

DWORD WINAPI listenerThread(LPVOID);

BOOL WINAPI ctrlCHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        if (isFirst) {        
            deathFlag = 1;            
            ReleaseSemaphore(firstSemaphore, 1, NULL);
            WaitForSingleObject(listenerThreadHandle, INFINITE);  

            CloseHandle(firstSemaphore);        
            CloseHandle(secondSemaphore);      

            CloseHandle(firstAcceptMessageEvent);        
            CloseHandle(secondAcceptMessageEvent);      

            CloseHandle(messagePort);     
        } else {
            deathFlag = 1;            
            ReleaseSemaphore(secondSemaphore, 1, NULL);
            WaitForSingleObject(listenerThreadHandle, INFINITE);    
        }

        printf("Messger was gracefully closed\n");
        exit(0);
    }
    return FALSE;
}


void establishConnection() {
    if (isFirst) {
        ReleaseSemaphore(secondSemaphore, 1, NULL);
        WaitForSingleObject(firstSemaphore, INFINITE);
        
    } else {
        ReleaseSemaphore(firstSemaphore, 1, NULL);
        WaitForSingleObject(secondSemaphore, INFINITE);
    }

    listenerThreadHandle = CreateThread( 
        NULL,                   // default security attributes
        0,                      // use default stack size  
        listenerThread,         // thread function name
        NULL,                   // argument to thread function 
        0,                      // use default creation flags 
        NULL);                  // returns the thread identifier    
}

void sendMessage(char* message) {
    long unsigned int writtenBytes;
    WriteFile(messagePort, message, BUFFER_SIZE, &writtenBytes, NULL);
    if (isFirst) {
        ReleaseSemaphore(secondSemaphore, 1, NULL);
    } else {
        ReleaseSemaphore(firstSemaphore, 1, NULL);
    }
    //printf("Waiting for acception...\n");
    if (isFirst) {
        WaitForSingleObject(firstAcceptMessageEvent, INFINITE);
        ResetEvent(firstAcceptMessageEvent);
    } else {
        WaitForSingleObject(secondAcceptMessageEvent, INFINITE);
        ResetEvent(secondAcceptMessageEvent);
    }   
    //printf("Got acception...\n");
}
void initialize() {
     if (!SetConsoleCtrlHandler(ctrlCHandler, TRUE)) {
        printf("\nERROR: Could not set control handler. Exit 1\n");
        exit(1); 
    }
    firstSemaphore = CreateSemaphore(NULL, 0, 1, FIRST_SEM);
    if (firstSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot create first semaphore, exit 2\n");
        exit(2);
    }
    firstAcceptMessageEvent = CreateEvent(
        NULL , 
        TRUE,   
        FALSE , 
        "firstGotMessage");

    secondSemaphore = CreateSemaphore(NULL, 0, 1, SECOND_SEM);
    if (secondSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot create pipe, exit 2\n");
        CloseHandle(firstSemaphore);
        exit(2);
    }
    secondAcceptMessageEvent = CreateEvent(
        NULL , 
        TRUE,   
        FALSE , 
        "secondGotMessage");

    if (isFirst) {
        messagePort = CreateFile(FIRST_PORT, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);        
    } else {
        messagePort = CreateFile(SECOND_PORT, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    }
    if (messagePort == INVALID_HANDLE_VALUE) {
        printf("Cannot open com-port, exit 3\n");
        CloseHandle(firstSemaphore);
        CloseHandle(secondSemaphore);
        exit(3);
    }
}

DWORD WINAPI listenerThread(LPVOID threadParam) { 
     while(1) {
        if (isFirst) {
            WaitForSingleObject(firstSemaphore, INFINITE);
        } else {
            WaitForSingleObject(secondSemaphore, INFINITE);
        }
        if (deathFlag != 0)  {
            printf("Listener thread was killed\n");
            return 0;
        }
        char message[BUFFER_SIZE];            
        long unsigned int readBytes;
        ReadFile(messagePort, message, BUFFER_SIZE, &readBytes, NULL);
        if (isFirst) {
            SetEvent(secondAcceptMessageEvent);
        } else {
            SetEvent(firstAcceptMessageEvent);
        }
        printf("\nGot message: %s", message);   
        printf("Enter message: ");
    }
    return 0; 
} 