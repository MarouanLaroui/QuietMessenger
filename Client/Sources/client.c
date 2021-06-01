#include "../Headers/client.h"

/*
connexionToServer :  -> Int
Create a socket
Connect it to server given in parameter
Returns an error if the creation failed
*/

int connexionToServer(int PORT){

    /*Creating a socket*/
	int dS = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in aS;
	aS.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &(aS.sin_addr));
	aS.sin_port = htons(PORT);

	/* Asking for a connection */
	socklen_t lgA = sizeof(struct sockaddr_in);
	int connectR = connect(dS, (struct sockaddr *) &aS, lgA);

	if (connectR == -1){
		perror("Une erreur est survenue lors de la connexion\n");
		exit(-1);
	}
    return dS;
}


/*
choixPseudo : threadData* -> void
Asking the entry of a pseudo until it is accepted by the server
If the pseudo is accepted, it is affected to the structure
Returns Void type
*/

void choixPseudo(struct threadData* data){

    int isPseudoAccepted = 0;

    while (!isPseudoAccepted){

        /* Allocating memory */
        char * pseudo = (char *) malloc(sizeof(char)*100);
        char * m = (char *) malloc(sizeof(char)*100);

        /* Entering a pseudo */
        printf("\nEntrez votre pseudo : ");
        pseudo = fgets(pseudo,100,stdin);
        int sendR = send(data->dS, pseudo, strlen(pseudo), 0);

        /* Checking if it was well sent */
        if (sendR == -1){
            perror("erreur\n");
            exit(-1);
        }
        /* Listening */
        int recvR = recv(data->dS, m, sizeof(char)*100, 0);

        /* Checking and printing errors */
        if (recvR == -1){
            perror("Une erreur est survenue lors de la réception du message.\n");
            exit(-1);
        }
        else{
            /* Checking the response */
            if(strcmp(m,"ok")==0){
                
                data->yourPseudo = pseudo;
                isPseudoAccepted = 1;
            }
            else{
                printf("\n%s",m);
            }
        }   

    }
}


/* Testing functions ======================================================== */


/*
isFileTransfer : String -> Int
if the message is "FILE", returns 1 
else returns 0
*/
int isFileTransfer(char *msg){
    
    if(strcmp(msg,"/file\n") == 0){
        return 1;
    }
    else{
        return 0;
    }
    
}


/*
isFileRequest : String -> Int
if the message is "FILEREQUEST", returns 1 
else returns 0
*/
int isFileRequest(char *msg){
    
    if(strcmp(msg,"/filerequest\n") == 0){
        return 1;
    }
    else{
        return 0;
    }
    
}


/* Sending functions ======================================================== */


/*
chooseFileToSend : Int -> String
Displays the list of the files availables
Asks for the entry of a file name by the user
Returns the file name
*/

char* chooseFileToSend(int dSClient){

    /* Displays the files */
    int cr = system("ls ./Client/Files");
    printf("\n");
    /* Checking */
    if(cr == -1){
        printf("commande echouée");
    }
    /* Asking for the name of the file to send */
    char* nomFichier = malloc(sizeof(char)*20);
    nomFichier = fgets(nomFichier,20,stdin);
    nomFichier[strcspn(nomFichier, "\n")] = 0;
    
    /* Sending the file name to the server and "l'info qu'il passe en mode serveur"   IDK WHAT IT MEANS*/
    int sendR = send(dSClient, nomFichier, strlen(nomFichier)+1, 0);
    if (sendR == -1){
        perror("Une erreur est survenue lors de l'envoi du message.\n");
        exit(-1);    
    }
    else{
        printf("Envoi du nom de fichier au serveur :  %s\n\n",nomFichier);
    }

    /* Building the file path with its name */
    char* filePath = malloc(sizeof(char)*30);
    strcpy(filePath,"Client/Files/");
    strcat(filePath,nomFichier);

    return filePath;
}


/*
upload_file : String -> Void
Takes the file path as parameter
Opens this file
Creates a file connection to the server
Sends the file to the server using the dedicated port
*/

void* upload_File(void* fDesciptor){
  long fd = (long) fDesciptor;
  int size = -1;
  char data[SIZE];
  int dSFile = connexionToServer(PORTFILE);

  /* While there is something to read*/
  while(size != 0) {

    size = read(fd,data,SIZE-1);
    
    /* Read the file and send the size of what has been read to the server*/
    send(dSFile,&size,sizeof(int),0);

    if(size!=0){
        if (send(dSFile, data, sizeof(data), 0) == -1) {

            perror("Erreur d'envoi du fichier.\n");
            exit(1);
        }
        printf("Le fichier s'envoie..\n\n");
    }
    
    /* Resetting the Buffer */
    bzero(data, SIZE);
    
  }

  printf("Le fichier est envoyé !\n\n");
  close(fd);
  shutdown(dSFile,2);
  pthread_exit(0);

}


/*
sendMessage "Void" -> Void
While the conversation is on, waits for a message entry by the user
Sends the message to the server
Returns Void type 
*/

void *sendMessage(void *receivedData){

    struct threadData *Data = (struct threadData*) receivedData;
    while(!Data->finConv){

        /* Allocating memory linked to the message */
        char * m = (char *) malloc(sizeof(char)*100);

        /* Entering message */
        m = fgets(m,100,stdin);

            /* Sending message */
            int sendR = send(Data->dS, m, strlen(m)+1, 0);
            if (sendR == -1){
                perror("Une erreur est survenue lors de l'envoi du message.\n");
                    exit(-1);
                
            }
            /*If it's a file transfer*/
            if(isFileTransfer(m)){

                /* Choosing the file name and sending it to the server */
                char* path = malloc(sizeof(char)*20);
                strcpy(path,chooseFileToSend(Data->dS));
                printf("Vérification si le fichier est conforme\n\n");
                long fd = open(path,O_RDONLY);
                if(fd==-1){
                    printf("Ce fichier n'existe pas\n");
                }
                else{
                    /* Creating the sending file thread */
                    pthread_t threadSendFile;
                    pthread_create(&threadSendFile,NULL,upload_File,(void*)fd);
                }
                
               
            }
            else{
                if(isFileRequest(m)){

                    printf("\nChoisissez un des fichiers suivants :\n\n");
                    /* Asking for the file name */
                    char* nomFichier = malloc(sizeof(char)*100);
                    fgets(nomFichier,100,stdin);
                    nomFichier[strcspn(nomFichier, "\n")] = 0;
                    /* Checking if the file is valid ??? */
                    int sendR = send(Data->dS, nomFichier, sizeof(char)*100, 0);
                    /* Creating a thread that will manage the file receiving */
                    pthread_t threadReceiveFile;
                    pthread_create(&threadReceiveFile,NULL,receiveFile,(void*)nomFichier);
                }
            }
        
        /* Realeasing the memory for the next message */
        free(m);
    }
    return NULL;
}



/* Receiving functions ======================================================== */



/*
receiveMessage : Void -> Void
While the conversation is on, waits for messages and
Displays messages if they are well received, else prints an error
Returns a Void type
*/

void *receiveMessage(void *receivedData){

    struct threadData *Data = (struct threadData*) receivedData;
    while(Data->finConv == 0){

        /*Allocating memory*/
        char * m = (char *) malloc(sizeof(char)*500);

        /* Listening */
        int recvR = recv(Data->dS, m, sizeof(char)*500, 0);

        /* Checking and printing errors */
        if (recvR == -1){
            perror("Une erreur est survenue lors de la réception du message.\n");
            exit(-1);
        }
        else{
            /*If the client socket is disconnected end the thread*/
            if(recvR == 0){
                printf("\n\nDéconnexion en cours...\n\n");
                free(m);
                Data->finConv = 1;
                pthread_exit(0);
                
            }
            else{
                printf("%s", m);
            }
            
         }

        /* Releasing memory */
        free(m);
    }    
    return NULL; 
}

/*
receiveFile : Void -> VoidF
Takes a file name as parameter
Dowloads the file 
*/

void *receiveFile(void* nomDuFichier){

    
    /* Creating connection to file server */
    int dSFile = connexionToServer(PORTFILE);
    char* nomFichier = (char*) nomDuFichier;

    /* Accepting connection to file transfer */
    int fd;
    int recvF;
    char buffer[SIZE];
    int sizeToWrite = -1;

    /* Recovering file name */
    char* cheminFichier = malloc(sizeof(char)*100);
    strcpy(cheminFichier,"Client/Files/");
    strcat(cheminFichier,nomFichier);

    /* Writing the file */  
    fd = open(cheminFichier, O_WRONLY | O_CREAT,0666);
    recv(dSFile, &sizeToWrite, sizeof(int), 0); 

    while(sizeToWrite !=0){

        /* Recovering file data and writing */
        recvF = recv(dSFile, buffer, SIZE, 0);
        write(fd,buffer,sizeToWrite);
        /* Restting the buffer */
        bzero(buffer,SIZE);
        /* Recovering how many bytes to receive from the server and writing */
        recv(dSFile, &sizeToWrite, sizeof(int), 0);
    }
    close(fd);
    printf("\nL'envoi de fichier est terminé\n\n");
    shutdown(dSFile,2);
    free(cheminFichier);
    pthread_exit(0);
}




/* Main client program */
int main(int argc, char *argv[]) {

    /* Checking number of arguments */
    if(argc!=3){
        perror("Veuillez lancer le programme avec ./client 162.38.111.181 8000\n");
    }
    IP = argv[1];
    PORT = atoi(argv[2]);
    PORTFILE = PORT + 1;

    /* Declaring and setting variables */
    pthread_t threadSend;
    pthread_t threadReceive;
    int finConv = 0;
    char* endWord = "/end\n";

    /* Creating a socket and connecting it */
    int dS = connexionToServer(PORT);

    /* Loading arguments of thread */
    struct threadData data;
    data.dS = dS;
    data.finConv = finConv;
    data.endWord = endWord;


	/*-----------------------------------COMMUNICATION-----------------------------------*/


    printf("\n\n\n--------------------- Conversation ---------------------\n\n");
    /* Choosing the pseudo */
    choixPseudo(&data);
    /* Creation of listening and sending message threads */
    pthread_create(&threadSend,NULL,sendMessage,&data);
    pthread_create(&threadReceive,NULL,receiveMessage,&data);
    /* Waiting for the end of threads */
    pthread_join(threadReceive,NULL);
    /* Secure closing */
    printf("Fermeture des threads..\n\n");
    pthread_cancel(threadSend);
    pthread_cancel(threadReceive);
    
    /* Closing the connection */
	shutdown(dS,2);

    printf("Vous êtes déconnecté ! \n\n");
    return 0;

}
