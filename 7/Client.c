#include "Messages.h"

void DieWithError(char *errorMessage);
char* GetClientServerMessage(ClientServerMessage *message);
char* GetServerClientMessage(ServerClientMessage *message);
void SendReadyMessage(int sock, enum ClientType clientType, int clientId);

int CreateSocket(const char *servIP, unsigned short echoServPort){
    int sock;
    struct sockaddr_in echoServAddr;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        DieWithError("socket() failed");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        DieWithError("connect() failed");
    }

    return sock;
}

enum Sign CreateRandomSign(){
    return (enum Sign) (rand() % 3 + 1);
}

void SendSignResponseMessage(int sock, int clientId) {
    ClientServerMessage signResponseMessage;
    signResponseMessage.id = rand() % 1000;
    signResponseMessage.clientType = PLAYER;
    signResponseMessage.clientId = clientId;
    signResponseMessage.message = SIGN_RESPONSE;
    signResponseMessage.sign = CreateRandomSign();

    if (send(sock, &signResponseMessage, sizeof(signResponseMessage), 0) != sizeof(signResponseMessage)) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    printf("%s\n", GetClientServerMessage(&signResponseMessage));
}


void PlayerWorker(int clientId, const char *servIP, unsigned short echoServPort) {
    int sock = CreateSocket(servIP, echoServPort);
    printf("Player with id %d started\n", clientId);
    SendReadyMessage(sock, PLAYER, clientId);

    for (;;) {
        ServerClientMessage serverClientMessage;

        if (recv(sock, &serverClientMessage, sizeof(ServerClientMessage), 0) < 0) {
            DieWithError("recv() failed");
        }

        printf("%s\n", GetServerClientMessage(&serverClientMessage));

        if (serverClientMessage.message == FINISH) {
            break;
        }

        if (serverClientMessage.message != SIGN_REQUEST) {
            DieWithError("Unexpected message");
        }

        SendSignResponseMessage(sock, clientId);
    }

    printf("Player with id %d finished\n", clientId);
    close(sock);
    exit(0);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));

    const char *servIP = argv[1];
    unsigned short echoServPort = atoi(argv[2]);

    for (int i = 0 ;i < NUM_PLAYERS; ++i) {
        pid_t pid = fork();
        if (pid == -1)
            DieWithError("fork() failed");
        else if (pid == 0) {
            PlayerWorker(i, servIP, echoServPort);
        }

        usleep(500000);
    }

    while (wait(NULL) > 0);

    return 0;
}
