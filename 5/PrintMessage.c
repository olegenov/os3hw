#include "Messages.h"

char* GetClientServerMessage(ClientServerMessage *message) {
    char *buffer = malloc(BUFFER_SIZE);
    sprintf(buffer, "ClientServerMessage: id=%d, ", message->id);

    switch (message->clientType) {
        case PLAYER:
            strcat(buffer, "clientType=PLAYER, ");
            break;
        default:
            strcat(buffer, "clientType=UNKNOWN, ");
            break;
    }

    char clientIdStr[50];
    sprintf(clientIdStr, "clientId=%d, ", message->clientId);
    strcat(buffer, clientIdStr);

    switch (message->message) {
        case READY:
            strcat(buffer, "message=READY, ");
            break;
        case SIGN_REQUEST:
            strcat(buffer, "message=SIGN_REQUEST, ");
            break;
        case SIGN_RESPONSE:
            strcat(buffer, "message=SIGN_RESPONSE, ");
            break;
        case FINISH:
            strcat(buffer, "message=FINISH, ");
            break;
        default:
            strcat(buffer, "message=UNKNOWN, ");
            break;
    }

    switch (message->sign) {
        case ROCK:
            strcat(buffer, "sign=ROCK");
            break;
        case PAPER:
            strcat(buffer, "sign=PAPER");
            break;
        case SCISSORS:
            strcat(buffer, "sign=SCISSORS");
            break;
        default:
            strcat(buffer, "sign=UNKNOWN");
            break;
    }

    return buffer;
}

char* GetServerClientMessage(ServerClientMessage *message) {
    char *buffer = malloc(BUFFER_SIZE);
    sprintf(buffer, "ServerClientMessage: id=%d, ", message->id);

    switch (message->clientType) {
        case PLAYER:
            strcat(buffer, "clientType=PLAYER, ");
            break;
        default:
            strcat(buffer, "clientType=UNKNOWN, ");
            break;
    }

    char clientIdStr[50];
    sprintf(clientIdStr, "clientId=%d, ", message->clientId);
    strcat(buffer, clientIdStr);

    switch (message->message) {
        case READY:
            strcat(buffer, "message=READY, ");
            break;
        case SIGN_REQUEST:
            strcat(buffer, "message=SIGN_REQUEST, ");
            break;
        case SIGN_RESPONSE:
            strcat(buffer, "message=SIGN_RESPONSE, ");
            break;
        case FINISH:
            strcat(buffer, "message=FINISH, ");
            break;
        default:
            strcat(buffer, "message=UNKNOWN, ");
            break;
    }

    strcat(buffer, "textMessage=");
    strcat(buffer, message->textMessage);

    return buffer;
}