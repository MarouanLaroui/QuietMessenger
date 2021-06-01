/******* LIB ********/


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 1024



/* Define the structure of args in parameters of a thread */
struct threadData{
    int dS;
    int finConv;
    char* endWord;
    char* yourPseudo;
};
int PORT;
char* IP;
int PORTFILE;
int dSFile;





/******* CONNECTION ********/


/*
connexionToServer :  -> Int
Create a socket
Connect it to server given in parameter
Returns an error if the creation failed
*/
int connexionToServer(int PORT);
/*
choixPseudo : threadData* -> void
Asking the entry of a pseudo until it is accepted by the server
If the pseudo is accepted, it is affected to the structure
Returns Void type
*/
void choixPseudo(struct threadData* data);





/******* COMMUNICATION ********/


/* Communication */

/* testing functions ======================================================== */

/*
isFileTransfer : String -> Int
if the message is "FILE", returns 1 
else returns 0
*/
int isFileTransfer(char *msg);
/*
isFileRequest : String -> Int
if the message is "FILEREQUEST", returns 1 
else returns 0
*/
int isFileRequest(char *msg);



/* Sending functions ======================================================== */

/*
chooseFileToSend : Int -> String
Displays the list of the files availables
Asks for the entry of a file name by the user
Returns the file name
*/
char* chooseFileToSend(int dSClient);
/*
upload_file : String -> Void
Takes the file path as parameter
Opens this file
Creates a file connection to the server
Sends the file to the server using the dedicated port
*/
void* upload_File(void* fDesciptor);
/*
sendMessage "Void" -> Void
While the conversation is on, waits for a message entry by the user
Sends the message to the server
Returns Void type 
*/
void *sendMessage(void *receivedData);


/* Receiving functions ======================================================== */

/*
receiveMessage : Void -> Void
While the conversation is on, waits for messages and
Displays messages if they are well received, else prints an error
Returns a Void type
*/
void *receiveMessage(void *receivedData);
/*
receiveFile : Void -> Void
Takes a file name as parameter
Dowloads the file 
*/
void *receiveFile(void* nomDuFichier);





/******* MAIN ********/


/* Main client program */
int main(int argc, char *argv[]);