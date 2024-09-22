#!/bin/bash
gcc Server.c DieWithError.c PrintMessage.c HandleClient.c AcceptConnection.c CreateServerSocket.c -o Server