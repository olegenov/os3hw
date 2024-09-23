#include "Messages.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

Player* players;
Viewer* viewers;
int** grid;
int shm_fd_players;
int shm_fd_grid;
int shm_fd_viewers;

void SendInfoMessageToAllViewers(char *textMessage) {
    for (int i = 0; i < MAX_VIEWERS; ++i) {
        if (viewers[i].sock == -1) {
            continue;
        }

        ServerClientMessage infoMessage;
        infoMessage.id = rand() % 1000;
        infoMessage.clientType = VIEWER;
        infoMessage.clientId = i;
        infoMessage.message = INFO;

        strcpy(infoMessage.textMessage, textMessage);
        
        if (viewers[i].sock != -1) {
            if (send(viewers[i].sock, &infoMessage, sizeof(infoMessage), 0) != sizeof(infoMessage)) {
                DieWithError("send() sent a different number of bytes than expected");
            }
        }
    }
}

void SigintHandler(int signum) {
    munmap(players, NUM_PLAYERS * sizeof(Player));
    close(shm_fd_players);
    shm_unlink("/players");

    munmap(grid, NUM_PLAYERS * NUM_PLAYERS * sizeof(int));
    close(shm_fd_grid);
    shm_unlink("/grid");

    munmap(viewers, MAX_VIEWERS * sizeof(Viewer));
    close(shm_fd_viewers);
    shm_unlink("/viewers");

    exit(0);
}

char* getPlayersScores() {
    char* result = malloc(1024);
    result[0] = '\0';
    int currentPlayers = 0;

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (players[i].score == -1) continue;
        currentPlayers += 1;
    }

    if (currentPlayers == 0) {
        return "No players\n";
    }

    for (int i = 0; i < currentPlayers; ++i) {
        char buffer[64];
        sprintf(buffer, "Player %d: %d\n", i, players[i].score);
        strcat(result, buffer);
    }

    strcat(result, "\n");

    return result;
}

char* getGrid() {
    char* result = malloc(1024);
    result[0] = '\0';
    int currentPlayers = 0;

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (players[i].score == -1) {
            continue;
        }
        currentPlayers += 1;
    }

    if (currentPlayers == 0) {
        return "No players\n";
    }

    sprintf(result, "\n%s |", result);

    for (int j = 0; j < currentPlayers; ++j) {
        sprintf(result, "%s%d ", result, j);
    }

    sprintf(result, "%s\n--", result);

    for (int j = 0; j < currentPlayers; ++j) {
        sprintf(result, "%s--", result);
    }

    sprintf(result, "%s\n", result);

    for (int i = 0; i < currentPlayers; ++i) {
        sprintf(result, "%s%d|", result, i);

        for (int j = 0; j < currentPlayers; ++j) {
            char buffer[16];
            sprintf(buffer, "%d ", grid[i][j]);
            strcat(result, buffer);
        }

        strcat(result, "\n");
    }

    return result;
}

void* printerWorker(void* arg) {
    while (1) {
        sleep(1);
        char buffer[1024];
        sprintf(buffer, "====================================\n");
        sprintf(buffer, "%sCurrent score state: %s", buffer, getPlayersScores());
        sprintf(buffer, "%sGrid: %s", buffer,getGrid());
        sprintf(buffer, "%s====================================\n",buffer);
        printf("%s", buffer);
        SendInfoMessageToAllViewers(buffer);
    }
}

void* handleClient(void* arg) {
    int clntSock = *((int *)arg);
    free(arg);
    HandleClient(clntSock, players, (int **) grid, viewers);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    signal(SIGINT, SigintHandler);
    signal(SIGTERM, SigintHandler);
    int servSock;
    unsigned short echoServPort;

    if (argc != 2) {
        fprintf(stderr,"Usage:  %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }
    shm_unlink("/players");
    shm_fd_players = shm_open("/players", O_CREAT | O_RDWR , 0666);
    if (shm_fd_players == -1) {
        perror("shm_open failed");
        exit(1);
    }

    if (ftruncate(shm_fd_players, NUM_PLAYERS * sizeof(Player)) == -1) {
        perror("ftruncate failed 1");
        exit(1);
    }
    shm_unlink("/grid");
    shm_fd_grid = shm_open("/grid", O_CREAT | O_RDWR, 0666);
    if (shm_fd_grid == -1) {
        perror("shm_open failed");
        exit(1);
    }

    if (ftruncate(shm_fd_grid, NUM_PLAYERS * NUM_PLAYERS * sizeof(int)) == -1) {
        perror("ftruncate failed 2");
        exit(1);
    }
    shm_unlink("/viewers");
    shm_fd_viewers = shm_open("/viewers", O_CREAT | O_RDWR, 0666);
    if (shm_fd_viewers == -1) {
        perror("shm_open failed");
        exit(1);
    }
    if (ftruncate(shm_fd_viewers, MAX_VIEWERS * sizeof(Viewer)) == -1) {
        perror("ftruncate failed 3");
        exit(1);
    }


    players = (Player *) mmap(0, NUM_PLAYERS * sizeof(Player), PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd_players, 0);
    if (players == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    grid = (int **) mmap(0, NUM_PLAYERS * NUM_PLAYERS * sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd_grid, 0);
    if (grid == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    viewers = (Viewer *) mmap(0, MAX_VIEWERS * sizeof(Viewer), PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd_viewers, 0);
    if (viewers == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        players[i].id = i;
        players[i].score = -1;
        players[i].isReady = false;
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        grid[i] = (int *) malloc(NUM_PLAYERS * sizeof(int));
        for (int j = 0; j < NUM_PLAYERS; ++j) {
            grid[i][j] = 8;
        }
    }

    for (int i = 0; i < MAX_VIEWERS; ++i) {
        viewers[i].sock = -1;
    }

    echoServPort = atoi(argv[1]);
    servSock = CreateServerSocket(echoServPort);

    pthread_t printerThread;
    if (pthread_create(&printerThread, NULL, printerWorker, NULL) != 0) {
        perror("pthread_create failed");
        exit(1);
    }

    for (;;) {
        int clntSock = AcceptConnection(servSock);
        int *new_sock = malloc(sizeof(int));
        *new_sock = clntSock;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handleClient, (void *)new_sock) != 0) {
            perror("pthread_create failed");
            continue;
        }
        pthread_detach(client_thread);
    }

    close(servSock);
    return 0;
}