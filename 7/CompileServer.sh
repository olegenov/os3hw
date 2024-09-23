#!/bin/bash
gcc Server.c DieWithError.c PrintMessage.c SendMessage.c HandleClient.c AcceptConnection.c CreateServerSocket.c -o Server