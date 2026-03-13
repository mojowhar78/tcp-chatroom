#ifndef _HEADER_
#define _HEADER_

#include <stdio.h>          // printf, scanf, perror
#include <stdlib.h>         // exit()
#include <string.h>         // memset, strcpy
#include <unistd.h>         // close()
#include <sys/types.h>      // data types
#include <sys/socket.h>     // socket, bind, listen, accept, connect, send, recv
#include <netinet/in.h>     // sockaddr_in, htons
#include <arpa/inet.h>      // inet_addr, inet_ntoa
#include<pthread.h>

#define SERVER_PORT 6333
#define LOGIN 2
#define SIGN_IN 1
#define LOG_OUT 3
#define MAX_CLIENT 5
#define ONLINE 10
#define OFFLINE 11
#define SUCCESS 12
#define FAILURE 13
#define WRONG_PASSWORD 14
#define USERNAME_NOT_FOUND 15
#define USERNAME_FOUND 16


#define MSG_NOTIFICATION  20
#define MSG_GROUP_CHAT    21
#define MSG_SINGLE_CHAT   22
#define MSG_ONLINE_LIST   23
#define MSG_VIEW_USERS    24
#define MSG_LOGOUT        25


typedef struct{
    
    char username[30];
    char password[10];
    short option;

}request_t;


typedef struct{

    char user[MAX_CLIENT][30];
    int count;

}onlinelist_t;


typedef struct{
    char sender[30];
    char receiver[30];
    char message[256];
    short type;
    
}msg_t;

#endif
