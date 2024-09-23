#include <semaphore.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define NUM_PLAYERS 3
#define BUFFER_SIZE 2048
#define MAX_VIEWERS 2

enum Sign {
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3
};

typedef struct {
    int id;
    int fd;
    int score;
    bool isReady;
} Player;

enum ClientType {
    PLAYER,
    VIEWER
};

enum Message {
    READY = 1,
    SIGN_REQUEST = 2,
    SIGN_RESPONSE = 3,
    FINISH = 4,
    INFO = 5
};

typedef struct {
    int id;
    enum ClientType clientType;
    int clientId;
    enum Message message;
    int sign;
} ClientServerMessage;

typedef struct {
    int id;
    enum ClientType clientType;
    int clientId;
    enum Message message;
    char textMessage[BUFFER_SIZE];
} ServerClientMessage;

typedef struct {
    int sock;
} Viewer;

void DieWithError(char *errorMessage);
int CreateServerSocket(unsigned short port);
int AcceptConnection(int servSock);
void HandleClient(int clientSocket, Player *players, int **grid, Viewer *viewers);
void SigintHandler(int signum);
char* GetClientServerMessage(ClientServerMessage *message);
char* GetServerClientMessage(ServerClientMessage *message);