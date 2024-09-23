#include "Messages.h"

void SendReadyMessage(int sock, enum ClientType clientType, int clientId) {
    ClientServerMessage readyMessage;
    readyMessage.id = rand() % 1000;
    readyMessage.clientType = clientType;
    readyMessage.clientId = clientId;
    readyMessage.message = READY;

    if (send(sock, &readyMessage, sizeof(readyMessage), 0) != sizeof(readyMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }
}

void SendFinishMessage(int sock, enum ClientType clientType, int clientId, char *textMessage) {
    ServerClientMessage finishMessage;
    finishMessage.id = rand() % 1000;
    finishMessage.clientType = clientType;
    finishMessage.clientId = clientId;
    finishMessage.message = FINISH;
    strcpy(finishMessage.textMessage, textMessage);

    if (send(sock, &finishMessage, sizeof(finishMessage), 0) != sizeof(finishMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }
}

void SendInformationMessage(int sock, enum ClientType clientType, int clientId, char *textMessage) {
    ServerClientMessage informationMessage;
    informationMessage.id = rand() % 1000;
    informationMessage.clientType = clientType;
    informationMessage.clientId = clientId;
    informationMessage.message = INFO;
    strcpy(informationMessage.textMessage, textMessage);

    if (send(sock, &informationMessage, sizeof(informationMessage), 0) != sizeof(informationMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }
}