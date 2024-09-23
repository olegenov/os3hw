#include "Messages.h"

int AcceptConnection(int servSock) {
    int clientSock;
    struct sockaddr_in echoClientAddr;
    unsigned int clientLen = sizeof(echoClientAddr);

    if ((clientSock = accept(servSock, (struct sockaddr *) &echoClientAddr, &clientLen)) < 0) {
        DieWithError("accept() failed");
    }

    printf("Handling client %s\n", inet_ntoa(echoClientAddr.sin_addr));

    return clientSock;
}
