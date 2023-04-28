#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 64
char name[MAX_NAME_LENGTH + 1];
char buffer[BUFFER_SIZE];
char gameBoard[3][3] = {{'.','.','.'}, {'.', '.', '.'}, {'.', '.', '.'}}; 
char clientmsg[2500];

struct sockaddr_in serveraddress;
int connectedClients = 0;
int gameStart = 0;
int numPlayers = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void *parsingclient(void* args_);
typedef struct {
    int socket;
    char assignment;
    char *name;
    int draw;
    int resigner;
    pthread_t thread;

} clientInfo;
struct socketTerm {
    int socket;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];
};

clientInfo clients[2];
char checkWinner(char board[3][3]) {
    //check rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == 'X' && board[i][1] == 'X' && board[i][2] == 'X') {
            return 'X';
        }
        else if (board[i][0] == 'O' && board[i][1] == 'O' && board[i][2] == 'O') {
            return 'O';
        }
    }
    //check columns
    for (int j = 0; j < 3; j++) {
        if (board[0][j] == 'X' && board[1][j] == 'X' && board[2][j] == 'X')
        {
            return 'X';
        }
        else if (board[0][j] == 'O' && board[1][j] == 'O' && board[2][j] == 'O')
        {
            return 'O';
        }

    }
    //check left diagonal 
    if (board[0][0] == 'X' && board[1][1] == 'X' && board[2][2] == 'X')
    {
        return 'X'; 
    }
    else if (board[0][0] == 'O' && board[1][1] == 'O' && board[2][2] == 'O')
    {
        return 'O'; 
    }
    if (board[0][2] == 'X' && board[1][1] == 'X' && board[2][0] == 'X')
    {
        return 'X';
    }
    else if (board[0][2] == 'O' && board[1][1] == 'O' && board[2][0] == 'O')
    {
        return 'O';
    }

    return '\0';
}

int boardFILL( char board[3][3])
{
    for (int i =0 ; i<3; i++)
    {
        for (int j =0; j<3; j++)
        {
            if ( board[i][j]!='O'&&board[i][j]!='X')
            {
                return 0; 
            }
        }
    }
    return 1;
}

int playgame(char board[3][3], char assignment, int x, int y) 
{  
    if (x < 1 || x > 3 || y < 1 || y > 3)
    {
        return 1; 
    }
    else if (board[x-1][y-1] == 'X' || board[x-1][y-1] == 'O')
    {
        
        return 1; 
    }
    else
    {
        board[x-1][y-1] = assignment;
        return 0; 
    }
    return 0; 
}

char* printboard(char board[3][3])
{
    char* pBoard = malloc(9 * sizeof(char));
    int x = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            pBoard[x] = board[i][j];
            x++;
        }
    }
    return pBoard;
}


void *parsingclient(void* args_)  
{

    pthread_mutex_lock(&lock);
    int numberClient = connectedClients;
    connectedClients++;
    pthread_mutex_unlock(&lock);
    if (numberClient == 0) {
        clients[numberClient].assignment = 'X'; 
    } else {
        clients[numberClient].assignment = 'O';
    }
    clients[numberClient].draw = 0;
    struct socketTerm* args = (struct socketTerm*) args_;
    int socketNew = args->socket;
    clients[numberClient].socket = socketNew;
    ssize_t numBytess = read(socketNew, args->buffer,BUFFER_SIZE);
    char begin[100];
    char *name;
    while (1)
    {
          
        if (numBytess == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        args->buffer[numBytess] = '\0';
        char *firstinput;
        char *rembuffer = malloc(strlen(args->buffer) + 1);
        memcpy(rembuffer, args->buffer, strlen(args->buffer) + 1);
        firstinput = strtok_r(rembuffer, "|", &rembuffer);


        char *numBytes = strtok_r(NULL, "|", &rembuffer); 

        if (strcmp(firstinput, "PLAY") == 0) {

            if (gameStart == 0) {
                int remainingChars = atoi(numBytes); //converts char* = "12" to int x= 12
                if (remainingChars < 2) {
                    char *error = "INVL|19|Must enter a name!|";
                    send(socketNew, error, strlen(error), 0); 
                } else if (remainingChars > MAX_NAME_LENGTH) {
                    char *error = "INVL|18|Name is too long!|";
                    send(socketNew, error, strlen(error), 0); 
                } else {

                    char *namePtr = malloc(strlen(args->buffer) + 1);
                    memcpy(namePtr, args->buffer, strlen(args->buffer) + 1);
                    char *strtokptr;
                    strtok_r(namePtr,"|",&strtokptr);
                    strtok_r(strtokptr,"|",&strtokptr);
                    name = strtok_r(strtokptr, "|", &strtokptr);
                    snprintf(begin, 100, "BEGN|%c|%s",clients[numberClient].assignment,name);
                    clients[numberClient].name = malloc(strlen(name) + 1);
                    strcpy(clients[numberClient].name, name);
                    numPlayers++;
                    if (numPlayers == 1) {
                        char *wait = "WAIT|0|";
                        send(socketNew, wait, strlen(wait), 0);
                    } else if (numPlayers == 2) {
                        char *wait = "WAIT|0|";
                        send(socketNew, wait, strlen(wait), 0);
                        for (int k = 0; k < 2; k++) {
                            snprintf(begin, 100, "BEGN|%c|%s|", clients[k].assignment,clients[k].name);
                            send(clients[k].socket, begin, strlen(begin), 0);
                            gameStart = 1;
                        }
                    }
                }
            } else {
            char *error = "INVL|15|Game has begun|";
            send(socketNew, error, strlen(error), 0);
            }
        }
        if (strcmp(firstinput, "RSGN")==0)
        {
            

            for (int k =0; k<2; k++)
            {
                close(clients[k].socket);
                
            }
             exit(EXIT_SUCCESS);
        }

        if (strcmp(firstinput, "MOVE") == 0)
        {
            // # of bytes should always be 6: 
            if (strcmp(numBytes, "6") != 0)
            {
                char *error = "INVL|26|Incorrect message length!|";
                send(socketNew, error, strlen(error), 0);
            }
            //assuming we get here, just get the assignment and the coordinates for the move
            char assignment = args->buffer[7];
            int firstcoordinate= args->buffer[9] - '0';
            int secondcoordinate = args->buffer[11] - '0';
            char (*boardPtr)[3] = gameBoard;
            if (playgame(boardPtr, assignment, firstcoordinate, secondcoordinate) == 1)
            {
                //invalid move error message
                char *error = "INVL|36|Space is occupied or does not exist|";
                send(socketNew, error, strlen(error), 0);
            }
            else if (boardFILL(boardPtr) == 1)
            {
                char *over = "OVER|15|D|Grid is full|";
                for (int i = 0; i < 2; i++)
                {
                    send(clients[i].socket, over, strlen(over), 0);
                     close(clients[i].socket);
                     
                }
                exit(EXIT_SUCCESS);
            }
            else
            {
                char winner = checkWinner(boardPtr);
                if (winner != '\0') 
                {
  
                    char winnerst[100]; 
                    char loserst[100];
                    char *winnerName = &assignment; 
                    int num = strlen(winnerName) + 19;
                    int bytesWritten1 = snprintf(winnerst, 100, "OVER|%d|W|Player %s has won.|", num, name);
                    int bytesWritten2 = snprintf(loserst, 100, "OVER|%d|L|Player %s has won.|", num, name);
                    if (bytesWritten1 < 0 || bytesWritten1 > 100 || bytesWritten2 < 0 || bytesWritten2 > 100)
                    {
                        perror("Snprintf error");
                        exit(EXIT_FAILURE);
                    }
                    for (int k = 0; k < 2; k++)
                    {
                        if(clients[k].assignment == assignment)
                        {
                            send(clients[k].socket, winnerst, strlen(winnerst), 0);
                        }
                        else 
                        {
                            send(clients[k].socket, loserst, strlen(loserst), 0);
                        }
                    }
                }
                else 
                {
                    
                    char statementMOVE[100];
                    int bytesWritten = snprintf(statementMOVE,100, "MOVD|16|%c|%d,%d|%s|", assignment, firstcoordinate, secondcoordinate, printboard(boardPtr));
                    if (bytesWritten < 0 || bytesWritten > 100)
                    {
                        perror("Snprintf error2");
                        exit(EXIT_FAILURE);
                    }
                    for (int k=0; k<2;k++)
                    {
                        send(clients[k].socket, statementMOVE, strlen(statementMOVE), 0);
                    }
                }
            }
        }
        if (strcmp(firstinput, "RSGN") == 0)
        {
            // # of bytes should always be 0: 
            if (strcmp(numBytes, "0") != 0)
            {
                char *error = "INVL|25|Incorrect message length|";
                send(socketNew, error, strlen(error), 0);
            }
            clients[numberClient].resigner = 1;

            char resignsss[100]; 
            char other[100]; 
            int num = strlen(name) + 24;
            
            int onebytes = snprintf(resignsss, 100, "OVER|%d|L|Player %s has resigned.|", num, name);
            int twobytes = snprintf(other, 100, "OVER|%d|W|Player %s has resigned.|", num, name);
            if (onebytes < 0 || onebytes > 100 || twobytes < 0 || twobytes > 100)
            {
                perror("Snprintf error");
                exit(EXIT_FAILURE);
            }
            for (int k = 0; k < 2; k++)
            {
                if(clients[k].resigner == 1 )
                {
                    send(clients[k].socket, resignsss, strlen(resignsss), 0);
                }
                else
                {
                    send(clients[k].socket, other, strlen(other), 0);
                }
                close(clients[k].socket);
                
            }
             exit(EXIT_SUCCESS);
            
        }

  if(strcmp(firstinput, "DRAW") == 0) 
        {
            char request = args->buffer[7];
            //int length = atoi(numBytes);
            if (request == 'S') 
            {
                for(int k = 0; k < 2; k++) 
                {
                    if (clients[k].socket != socketNew) 
                    {
                        clients[k].draw = 1;
                    }
                }
                for(int k = 0; k < 2; k++) 
                {
                    if (clients[k].socket != socketNew) 
                    {
                        send(clients[k].socket, "DRAW|2|S|", strlen(args->buffer), 0);
                    }
                }
            } 
            else if (request == 'A')
            {
                if(clients[numberClient].draw == 1) 
                {
                    char response[100];
                    snprintf(response, 100, "OVER|21|D|Draw was accepted.|");
                    for (int k = 0; k < 2; k++) {
                        send(clients[k].socket, response, strlen(response), 0);
                    }
                    for (int i = 0; i < 2; i++) {
                        close(clients[i].socket);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    char *error = "INVL|18|Draw was not sent|";
                    send(socketNew, error, strlen(error), 0);
                }
            } else if (request == 'R') {
                if (clients[numberClient].draw == 1) {
                    char response[100];
                    snprintf(response, 100, "DRAW|2|R|");
                    for (int k = 0; k < 2; k++) {
                        if (k != numberClient) {
                            send(clients[k].socket, response, strlen(response), 0);
                        }
                    }
                    for(int k = 0; k < 2; k++) {
                        clients[k].draw = 0;
                    }
                } 
                else 
                {
                    char *error = "INVL|18|Draw was not sent|";
                    send(socketNew, error, strlen(error), 0);
                }
            } 
            else {
                char *error = "INVL|27|Invalid DRAW request type.|";
                send(socketNew, error, strlen(error), 0);
            }

        }
        numBytess = read(socketNew, args->buffer,BUFFER_SIZE);
    }
}

int main (int argc , char *argv[])
{
    pthread_t thread_id;
    int sSocket;//server socket
    int socketNew;
    if (argc < 2)
    {
        printf("missing port\n");
        return -1;
    }
    socklen_t addressSize;
    struct sockaddr_storage serverstorage;
    sSocket = socket(AF_INET, SOCK_STREAM, 0);

    //failed to create the socket we requested
    
    if (setsockopt(sSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))<0)
    {
        perror("Reusing socket failed");
    }
    if (sSocket==0)
    {
        perror("Could not create socket");
        exit (EXIT_FAILURE);
    }

    char IP[25];
    char newIP[25];
    IP[24]='\0';

    gethostname(IP,25);
    struct hostent *host;
    host = gethostbyname(IP);

    snprintf (newIP, 25, "%s", inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));


    serveraddress.sin_family= AF_INET;
    serveraddress.sin_port=htons(atoi(argv[1]));
    serveraddress.sin_addr.s_addr=inet_addr(newIP);
    memset(serveraddress.sin_zero, '\0', sizeof(serveraddress.sin_zero));

 
    if (bind(sSocket, (struct sockaddr*)&serveraddress, sizeof(serveraddress))<0)
    {
        perror ("Socket Could not bind");
        exit (EXIT_FAILURE);
    }
    //max connections st to 10
    if (listen(sSocket,10)<0)
    {
        perror("sSocket could not listen");
        exit(EXIT_FAILURE);
    }

    addressSize=sizeof(serverstorage);
    while (1) 
    {
        socketNew = accept(sSocket, (struct sockaddr*) &serveraddress, &addressSize);
        if (socketNew > 0) 
        {
            struct socketTerm* args = malloc(sizeof(struct socketTerm));
            args->socket = socketNew;
            args->addr = serveraddress;
            pthread_create(&thread_id, NULL, &parsingclient, (void*) args);
            clients[connectedClients].thread = thread_id;
        } 
        else 
        {
            perror("Failed to accept connection");
        }
    }
      for(int k = 0; k < 2; k++) {
        close(clients[k].socket);
        pthread_cancel(clients[k].thread);
    }
     exit(EXIT_SUCCESS);
}