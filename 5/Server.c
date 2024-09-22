#include "Messages.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

Player* players;
int** grid;
int shm_fd_players;
int shm_fd_grid;

void SigintHandler(int signum) {
    munmap(players, NUM_PLAYERS * sizeof(Player));
    close(shm_fd_players);
    shm_unlink("/players");

    munmap(grid, NUM_PLAYERS * NUM_PLAYERS * sizeof(int));
    close(shm_fd_grid);
    shm_unlink("/grid");

    exit(0);
}

char* getPlayersScores() {
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
        printf("====================================\n");
        printf("Current score state: %s", getPlayersScores());
        printf("Grid: %s", getGrid());
        printf("====================================\n");
    }
}

void* handleClient(void* arg) {
    int clntSock = *((int *)arg);
    free(arg);
    HandleClient(clntSock, players, (int **) grid);
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

    shm_fd_players = shm_open("/players", O_CREAT | O_RDWR, 0666);
    if (shm_fd_players == -1) {
        perror("shm_open failed");
        exit(1);

    }

    if (ftruncate(shm_fd_players, NUM_PLAYERS * sizeof(Player)) == -1) {
        perror("ftruncate failed");
        exit(1);
    }

    shm_fd_grid = shm_open("/grid", O_CREAT | O_RDWR, 0666);
    if (shm_fd_grid == -1) {
        perror("shm_open failed");
        exit(1);
    }

    if (ftruncate(shm_fd_grid, NUM_PLAYERS * NUM_PLAYERS * sizeof(int)) == -1) {
        perror("ftruncate failed");
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
        pthread_detach(client_thread);  // Отсоединение потока для автоматической очистки ресурсов
    }

    close(servSock);
    return 0;
}