#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define SIZE 1024
#define NBOFROOMS 3
#define SIZEOFROOMNAME 20
#define SIZEOFPSEUDO 15
#define SIZEOFMESSAGE 100
#define SIZEOFDESCR 150
#define SIZEOFPSWD 20




/* Global variables */

int nbClient;
int nbClientMax;
int PORT;
int PORTFILE;
int dSFile;
int nbOfRoomDisplayed;
int dS;
int dSFile;


sem_t semaphoreNbClient;

/*Mutex*/
pthread_mutex_t mutex_tab  = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t mutex_rooms  = PTHREAD_MUTEX_INITIALIZER;;

/*Structures*/
struct dataClient{

    int dS;
    char* pseudo;
    char* password;
    int roomId;
    int roleTab[NBOFROOMS];
    int nbOfWarnings;
    int isAdmin;
};

struct room{

    int id;
    char* name;
    int nbOfMembers;
    int maxMembers;
    int members[5];
    int isDisplayed;
    char* description;
    
};

typedef struct dataClient dataClient;
typedef struct room room;

struct dataClient tabClient[5];
struct room roomsList[NBOFROOMS];
pthread_t tabThread[5];


/* Connection */

/*
firstIndexEmpty : int[] x Int -> Int
Returns the index of the first case of the table, given in parameter, that contains -1, if it exists,
else returns -1
*/
int firstIndexEmpty(struct dataClient* tab, int taille);


/*
isPseudoAvailable : String x dataClient[] x Int
If the pseudo is not contained into the table structure return 1,
else returns 0
*/
int isPseudoAvailable(char* pseudo);


/*
selectPseudo : Int x dataClient[] x taille
Waits a pseudo entry
Checks if it is available
*/
void selectPseudo(int currentClientDS);


/*
findClientByPseudo : String -> Client
Takes a pseudo as parameter
Checks if a client already has this pseudo
If the pseudo is available returns -1
else returns the client socket descripter
*/
int findDSClientByPseudo(char* pseudo);


/*
findIndexByDS : Int -> Int
Return the index of the client found by it's dS
Return -1 if not found
*/
int findIndexByDS(int dS);

/*
findPasswordByPseudo : String -> String
return the password of a client given in parameter by its pseudo
*/
char* findPasswordByPseudo(char* pseudo);

/*
loginUser: Int x String
ask for the client given in parameter a password 
Return 1 : if the password matches with the one in DB
Return 0 : if not
*/
int loginUser(int indexClient,char* password);


/*
registerUser : Int -> Void
ask the client in parameter for a password 
write in the DB the client infos
*/
void registerUser(int indexClient);

/*
removeClientFromDB : String -> Void
remove the client given in parameter from the DB
*/
void removeClientFromDB(char* pseudo);

/* Communication */

/* Testing functions ======================================================== */

/*
isCommand : char* -> int
Returns 1 if it is a command
Else returns 0
*/
int isCommand(char *msg);


/*
hasInsult : String -> int
Return 1 if the message has banWords in it
Returns 0 if not
*/
int hasInsult(char* msgToTreat);


/*
hasInsultTreatement : Int -> Void
gives warnings to the client if he uses ban words
and keep count of the nb of warnnings
at the third warning he's banned
*/
void hasInsultTreatement(int index);


/*
isClientRegistered : Int -> Void
Return 0 if the client isn't registered,
Return 1 if the is found in the database and stock it's password
*/
int isClientRegistered(char* pseudo,char* password);


/* Setting functions ======================================================== */

/*
createServerSocket : Int -> Int
Creates a socket on the port given in parameter
Makes it listen
Returns the socket descripter created
*/
int createServerSocket(int port);

char* extractPseudo(char *msg);
char* extractAttributeN(int n, char* msg,char* separator);
char* extractPrivateMessage(char* msg);


/*
initClientFromDB : Int x String -> 
Look for a client in the database that matches the parameter and init the client from it
Return : Void
*/
void initClientFromDB(int index, char* pseudo );


/*
updateClientToDB : Int x String -> Void
Update clients infos in the DB
Return : void
*/
void updateClientFromDB(char* pseudo);

/*
messageTreatement : String,Int -> Void
Adds the sender pseudo to the message and shapes it
Returns Void type
*/
void messageTreatement(char* msg,int i);


/*
reinitCellTab : int[] x int -> Void
Resets the first case (fills by -1) of a table that contains the int given in parameter
Returns Void type
*/
void reinitCellTab(int dS);


/*
getFileSendable : void* -> void*
Takes a client socket descripter as parameter
Sends to it the list of files that it can download 
Waits the reception of the file name that the client wants
Launch the function that sends the file
Returns NULL
*/
void* getFileSendable(void* dSC);


/*
encodeCommand : String -> Int
check if the message contains a known command 
and return a specific value unique to each command
*/
int encodeCommand(char *msg);

/* room settings */

/*
initRooms : String -> Void
Initialize the different rooms with it past data (name,files...)
*/
void initRooms();

/*
changePseudo : Int -> Void
Ask the user for a new pseudo and check if it's already in use :
if not : set as new pseudo
Return : Void
*/
void changePseudo(int index);
void createRoom(int clientIndex);


/*
closeRoom : Int x String -> Void
Close and free the space needed by the rooms
*/
void closeRoom(int clientIndex,char* roomName);


/*
shutdownRooms : Void -> Void
Close and free the space needed by the rooms
*/
void shutdownRooms();


/*
displayRooms : Void -> Void
print the different rooms and their members 
*/
void displayRooms();


/*
displayRoomsToClient : Void -> Void
send the different rooms and their members to the client
*/
void displayRoomsToClient(int clientIndex);


/*
displayCommandToClient : Int -> Void
sends every command to the client from a txt file
*/
void displayCommandToClient(int indexC);


/*
firstIdRoomNotDisplayed : Void -> Int
return the first id of a not room displayed room in roomList array 
*/
int firstIdRoomNotDisplayed();

/*
findIDRByIndex : String -> Client
Return IDR of a client given by it's Index
*/
int findIDRByIndex(int index);


/*
findIdRoomByName : String -> Int
Return the Id of a room given by it's name
*/
int findIdRoomByName(char* roomName);


/*
isRoomNameAvailable : -> Bool
Return true if the name of the room isn't used by other rooms
Return false if used
*/
int isRoomNameAvailable(char* name);


/*
joinRoom :Int x String -> Void
Verify if the room exists and if it has room for the client, if it does connect the client to the room
Else send an error to the client
*/
int joinRoom(int clientIndex,char* roomName);

/*
changeRoomName : Int x String -> Void
Check if the client has the right to modify room name
if so ask the client for a new description and changes it
if not send error msg
*/
void changeRoomName(int clientIndex,char* Oldname);


/*
changeRoomDescription : Int x String -> Void
Check if the client has the right to modify room description
if so ask the client for a new description and changes it
if not send error msg
*/
void changeRoomDescription(int clientIndex,char* roomName);


/*
saveRoomInfo : Void -> Void
save in the file all of the room infos
*/
void saveRoomInfo();

/*
exitRoom : Void -> Void
save in the file all of the room infos
*/
void exitRoom(int clientIndex,char* roomName);


/*
setModerator : Int x String x String -> Void
check if the client has the rights to set someone mod
check if the other client is in the room 
set 
*/
void setModeratorOf(int indexAdmin,char* pseudo,char* roomName);

/*
isModeratorOf : Int x String -> Int
check if the client is moderator of the room given in parameter*
return 1 if he is 
else return 0 
*/
int isModeratorOf(int indexClient,char* roomName);


/*
setAdminServer : Int x String -> Void
check if the client has the rights to set someone mod
check if the other client is in the room 
set 
*/
void setAdminServer(int indexAdmin,char* pseudo);


/*
isAdminOf : Int x String -> Int
check if the client is admin of the room given in parameter
return 1 if he is 
else return 0 
*/
int isAdminOf(int indexClient,char *roomName);

/*
isAdminServer : Int -> Int 
check if the client is an Admin in the client's structure
return 1 if he is 
else return 0 
*/

int isAdminServer(int indexClient);


/*
whoIsHere : Int -> void
Display to the client given in parameter the ones that are online
*/
void whoIsHere(int clientIndex);


/*
setRandomInRoom : Int x String x String
check if the client has the rights to set someone mod
check if the other client is in the room 
set 
*/
void setRandom(int indexAdmin,char* pseudo,char* roomName);
int hasMoreRights(int index1, int index2, int idR);

/*
moveClient : Int -> Void
ask the user in parameter for a client and a room to move the client he chooses to in it
*/
/*TODO*/
void moveClient(int clientIndex);

/*
disconnectClient: Int -> Void
disonnect a client, and reinit the client's structure to default
*/
void disconnectClient(int clientIndex);
/*
kickTreatement : Int -> Void 
Ask the client in parameter for a client to kick it from the server
Return : Void
*/
void kickTreatement(int clientIndex);


/*
kickFromServer : String -> Void
kick the client given in parameter from the Server
Return 1 : success
Return 0 : failure
*/
int kickFromServer(char* pseudo);


/*
kickFromRoom : String -> String
Kick the client from the room given in parameter
Return 1 : is a success
Return 0 : failure
*/
int kickFromRoom(char* pseudo,char* roomName);


/*
kickFromRoomTreatment : Int -> Void
Ask the client in parameter for a room and a client to expulse
Return : Void
*/
void kickFromRoomTreatment(int clientIndex);


/*
unBanTreatment : Int -> Void
Ask the client in parameter for a client to unban from the server
send messages related to the status of this operation
*/
void unBanTreatment(int clientIndex);


/*
banTreatment : Int
Ask the client in parameter for a client to ban from the server
send messages related to the status of this operation
*/
void banTreatment(int clientIndex);


/*
registerClient : String x String - >
register a new client to the Client FILE
*/
void registerClientToDB(int indexClient,char* password);

/*
exitRoomTreatment : Int -> Void
Interact with the client to know
*/
void exitRoomTreatement(int clientIndex);


/*
exitRoomTreatment : Int -> Void
Interact with the client to know
*/
void deleteRoomTreatement(int clientIndex);

/*
promoteToAdminTreatement : Int -> Void
Interact with the client to know which user to promote and in wich room
Call setAdmin func
*/
void promoteToAdminTreatement(int clientIndex);

/*
promoteToModTreatement : Int -> Void
Interact with the client to know which user to promote and in which room
Call the function setAdmin
*/
void promoteToModTreatement(int clientIndex);

/*
demoteTreatement : Int -> Void
Interact with the client to demote and call setRandom
*/
void demoteTreatement(int clientIndex);

/*
deleteRoomTreatement : Int -> Void
Interact with the client to know wich room he wants to delete
and call the delete function
*/
void deleteRoomTreatement(int clientIndex);

/*
renameRoomTreatment : Int -> Void
Interact with the client to know wich room he wants to delete
and rename the room
*/
void renameRoomTreatement(int clientIndex);


/*
joinRoomTreatment
ask the client for a room to join and send a message related to the status of the operation
*/
void joinRoomTreatement(int clientIndex);


/*
blackListClient : String -> Void
add the Client given in parameter from the blackList
Return : Void 
*/
void blackListClient(char* pseudoToBan);


/*
unblackListClient : String -> Void
remove the Client given in parameter from the blackList
Return : Void
*/
void unBlackListClient(char* pseudoToUnban);

/*
isClientBlackListed : String x String -> Int
Return 0 if the client isn't isClientBlackListed,
Return 1 if isClientBlackListed
*/
int isClientBlackListed(char* pseudo);

/* Sending functions ======================================================== */


/*
send_file : String -> Void
Takes the file path as parameter
Sends the file to the client
Returns Void type
*/
void send_file(char* filePath);


/*
sendMessageToAll : int x char[] x int[] x int -> Void
Sends the message given in parameter to all clients in the main room except the author of the message (given in parameter)
If it works, prints "ok" and quits the function
else prints the error
Returns Void type
*/
void sendMessageToAll(int dSC, char * msg);


/*
sendMessageTo : int x char[] -> Void
Sends the message given in parameter to the client given in parameter
If it works, prints "ok" and quits the function
else prints the error
Returns Void type
*/
void sendMessageTo(int dSC,char*msg);


/*
sendMessageToRoom : int x char[] -> Void
Sends the message given in parameter to the client given in parameter
If it works, prints "ok" and quits the function
else prints the error
Returns Void type
*/
void sendMessageToRoom(int clientIndex,char*msg);


/*
getFeedBackFrom : String x String -> Void
Sends a message to the client given in parameter and wait for its response
*/
void getFeedBackFrom(int clientIndex,char* msgToSend, char* msgBuffer);


/* Receiving functions ======================================================== */


/*
receiveMessageFrom : int x char[] -> Void
Waits and listens to a message of a client given in parameter
If error, quit the function and prints the error
else displays the message reveived
Returns int
*/
int receiveMessageFrom(int dSC, char* msg);


/*
receiveFile : void* -> void*
Takes a file name as parameter
Sets the connexion with a file sender
Waits the reception of the file
Receives the file 
If errors, prints it
Returns NULL
*/
void* receiveFile(void* nomFichier);



/* Running functions ======================================================== */


/*
transfert : "Void" -> Void
Listens to a client and transfer its messages to all other clients
Returns Void type
*/
void *transfert(void* indice);

int executeCommand(int clientIndex,long dSC,int rcv,pthread_t threadReceiveFile,pthread_t threadSendFile,char* msg);