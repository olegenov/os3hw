cmake_minimum_required(VERSION 3.27)
project(5 C)

set(CMAKE_C_STANDARD 11)

add_executable(5
        Messages.h
        AcceptConnections.c
        Client.c
        CreateServerSocket.c
        HandleClient.c
        DieWithError.c
        PrintMessage.c
        Server.c)
