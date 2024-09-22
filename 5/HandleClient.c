#include "Messages.h"

void DieWithError(char *errorMessage);
char* GetClientServerMessage(ClientServerMessage *message);
char* GetServerClientMessage(ServerClientMessage *message);

bool AddPlayer(Player *players, int clientId, int clntSocket) {
    if (clientId < 0 || clientId >= NUM_PLAYERS) {
        return false;
    }

    players[clientId].score = 0;
    players[clientId].fd = clntSocket;
    players[clientId].isReady = true;

    return true;
}

bool RemovePlayer(Player *players, int clientId, int **grid) {
    if (clientId < 0 || clientId >= NUM_PLAYERS) {
        return false;
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (players[i].score == -1) {
            continue;
        }

        grid[clientId][i] = 8;
        grid[i][clientId] = 8;
    }

    players[clientId].score = -1;
    close(players[clientId].fd);

    return true;
}

void SendFinishMessage(int sock, int clientId, char *textMessage) {
    ServerClientMessage finishMessage;
    finishMessage.id = rand() % 1000;
    finishMessage.clientType = PLAYER;
    finishMessage.clientId = clientId;
    finishMessage.message = FINISH;
    strcpy(finishMessage.textMessage, textMessage);

    if (send(sock, &finishMessage, sizeof(finishMessage), 0) != sizeof(finishMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    printf("%s\n", GetServerClientMessage(&finishMessage));
}

void SendSignRequestMessage(int sock, int clientId) {
    ServerClientMessage signRequestMessage;
    signRequestMessage.id = rand() % 1000;
    signRequestMessage.clientType = PLAYER;
    signRequestMessage.clientId = clientId;
    signRequestMessage.message = SIGN_REQUEST;
    sprintf(signRequestMessage.textMessage, "Please send your sign");

    if (send(sock, &signRequestMessage, sizeof(signRequestMessage), 0) != sizeof(signRequestMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    printf("%s\n", GetServerClientMessage(&signRequestMessage));
}

void HandleClient(int clntSocket, Player *players, int **grid) {
    ClientServerMessage clientServerMessage;
    if (recv(clntSocket, &clientServerMessage, sizeof(ClientServerMessage), 0) < 0) {
        DieWithError("recv() failed");
    }

    if (clientServerMessage.message != READY) {
        DieWithError("First message should be READY");
    }

    bool isAddPlayer = AddPlayer(players, clientServerMessage.clientId, clntSocket);

    if (!isAddPlayer) {
        SendFinishMessage(clntSocket, clientServerMessage.clientId, "Server is full");
        close(clntSocket);

        return;
    }

    printf("Client with id %d is ready\n", clientServerMessage.clientId);

    for (;;) {
        int currentPlayersCount = 0;

        for (int i = 0; i < NUM_PLAYERS; ++i) {
            if (players[i].score == -1) {
                continue;
            }
            currentPlayersCount += 1;
        }

        if (currentPlayersCount == NUM_PLAYERS) {
            break;
        }
    }

    for (;;) {
        if (players[clientServerMessage.clientId].isReady == false) {
            continue;
        }

        int donePlayersCount = 0;

        for (int i = 0; i < NUM_PLAYERS; ++i) {
            if (i == clientServerMessage.clientId) {
                continue;
            }

            if (players[i].score == -1) {
                continue;
            }

            if (grid[i][clientServerMessage.clientId] != 8) {
                donePlayersCount += 1;
                continue;
            }

            if (players[i].isReady == false) {
                continue;
            }

            players[clientServerMessage.clientId].isReady = false;
            players[i].isReady = false;
            SendSignRequestMessage(players[i].fd, i);
            SendSignRequestMessage(players[clientServerMessage.clientId].fd, clientServerMessage.clientId);
            ClientServerMessage clientServerMessageFromCurrentPlayer;

            if (recv(players[i].fd, &clientServerMessageFromCurrentPlayer, sizeof(ClientServerMessage), 0) < 0) {
                DieWithError("recv() failed");
            }

            ClientServerMessage clientServerMessageFromOtherPlayer;

            if (recv(clntSocket, &clientServerMessageFromOtherPlayer, sizeof(ClientServerMessage), 0) < 0) {
                DieWithError("recv() failed");
            }

            if (clientServerMessageFromCurrentPlayer.message != SIGN_RESPONSE || clientServerMessageFromOtherPlayer.message != SIGN_RESPONSE) {
                DieWithError("Unexpected message");
            }

            printf("%s\n", GetClientServerMessage(&clientServerMessageFromCurrentPlayer));
            printf("%s\n", GetClientServerMessage(&clientServerMessageFromOtherPlayer));

            if (clientServerMessageFromCurrentPlayer.sign == clientServerMessageFromOtherPlayer.sign) {
                players[i].score += 1;
                players[clientServerMessage.clientId].score += 1;
                grid[i][clientServerMessage.clientId] = 1;
                grid[clientServerMessage.clientId][i] = 1;
            } else if (clientServerMessageFromCurrentPlayer.sign == ROCK &&
                       clientServerMessageFromOtherPlayer.sign == SCISSORS) {
                players[clientServerMessage.clientId].score += 2;
                grid[clientServerMessage.clientId][i] = 2;
                grid[i][clientServerMessage.clientId] = 0;
            } else if (clientServerMessageFromCurrentPlayer.sign == PAPER &&
                       clientServerMessageFromOtherPlayer.sign == ROCK) {
                players[clientServerMessage.clientId].score += 2;
                grid[clientServerMessage.clientId][i] = 2;
                grid[i][clientServerMessage.clientId] = 0;
            } else if (clientServerMessageFromCurrentPlayer.sign == SCISSORS &&
                       clientServerMessageFromOtherPlayer.sign == PAPER) {
                players[clientServerMessage.clientId].score += 2;
                grid[clientServerMessage.clientId][i] = 2;
                grid[i][clientServerMessage.clientId] = 0;
            } else {
                players[i].score += 2;
                grid[i][clientServerMessage.clientId] = 2;
                grid[clientServerMessage.clientId][i] = 0;
            }

            players[i].isReady = true;
            players[clientServerMessage.clientId].isReady = true;
            sleep(1);
        }

        if (donePlayersCount == NUM_PLAYERS - 1) {
            break;
        }
    }

    SendFinishMessage(clntSocket, clientServerMessage.clientId, "Game finished");
    printf("Client with id %d finished\n", clientServerMessage.clientId);
//    RemovePlayer(players, clientServerMessage.clientId, grid);
}
