#include "Messages.h"

void DieWithError(char *errorMessage);  /* Error handling function */
char* GetClientServerMessage(ClientServerMessage *message);
char* GetServerClientMessage(ServerClientMessage *message);
void SendReadyMessage(int sock, enum ClientType clientType, int clientId);
void SendFinishMessage(int sock, enum ClientType clientType, int clientId);

int sock;

void SigintHandler(int signum) {
    ClientServerMessage clientServerMessage;
    clientServerMessage.id = rand() % 1000;
    clientServerMessage.clientType = VIEWER;
    clientServerMessage.clientId = 0;
    clientServerMessage.message = FINISH;
    clientServerMessage.sign = signum;

    if (send(sock, &clientServerMessage, sizeof(clientServerMessage), 0) != sizeof(clientServerMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    printf("%s\n", GetClientServerMessage(&clientServerMessage));
    exit(0);
}

int createSocket(const char *servIP, unsigned short echoServPort){
    int sock;
    struct sockaddr_in echoServAddr;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        DieWithError("connect() failed");
    }

    return sock;
}


void ViewWorker(const char *servIP, unsigned short echoServPort, int viewer_id) {
    sock = createSocket(servIP, echoServPort);
    printf("Connected to server\n");
    SendReadyMessage(sock, VIEWER, viewer_id);

    while (1) {
        ServerClientMessage serverClientMessage;
        int bytesRcvd = recv(sock, &serverClientMessage, sizeof(ServerClientMessage), 0);

        if (bytesRcvd <= 0) {
            continue;
        }

        printf("Received:\n%s\n", serverClientMessage.textMessage);

        if (serverClientMessage.message == FINISH) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    signal(SIGINT, SigintHandler);
    signal(SIGTERM, SigintHandler);

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n", argv[0]);
        exit(1);
    }

    const char *servIP = argv[1];
    unsigned short echoServPort = atoi(argv[2]);

    ViewWorker(servIP, echoServPort, 0);
    close(sock);
    while (wait(NULL) > 0);

    return 0;
}
