#include "header.h"

typedef struct
{
    char user_name[30];
    char password[10];
    int status;
    int socket_fd;

} client_info;

client_info clients[MAX_CLIENT];
int client_no;
pthread_mutex_t lock;

void *client_handler(void *arg);
void authentication(int client_fd);
void send_online_users(int client_fd);
void broadcast_notification(char *username, int status);
void single_chat(msg_t *msg);
void group_chat(msg_t * msg);
void message_handle(int client_fd,char *username);
void logout(char *username,int client_fd);

/*------------------------------------------>>MAIN<<------------------------------------------------------*/
int main()
{

    if (pthread_mutex_init(&lock, NULL) != 0)
    {

        printf("\nPthread: mutex init failed\n");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_info;

    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(SERVER_PORT);
    server_info.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (const struct sockaddr *)&server_info, sizeof(server_info)) == -1)
    {
        perror("BIND");
        exit(1);
    }

    listen(server_fd, MAX_CLIENT);
    printf("SERVER_LISTENING:\n");

    while (1)
    {
        int c_fd = accept(server_fd, NULL, 0);
        int *client_fd = malloc(sizeof(int));
        *client_fd = c_fd;
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid);
    }
}

void *client_handler(void *arg)
{

    int client_fd = *(int *)arg;
    free(arg);
    authentication(client_fd);
    send_online_users(client_fd);

    char username[30];
    
    pthread_mutex_lock(&lock);
    for(int i = 0; i < client_no; i++){
        if(clients[i].socket_fd == client_fd){
            strcpy(username, clients[i].user_name);
            break;
        }
    }

    pthread_mutex_unlock(&lock);
    broadcast_notification(username,ONLINE);
    message_handle(client_fd,username);


}

void send_online_users(int client_fd){

    onlinelist_t list;
    memset(&list, 0, sizeof(list));

    pthread_mutex_lock(&lock);
    for(int i = 0; i < client_no; i++){
        if(clients[i].status == ONLINE){
            strcpy(list.user[list.count++],
                   clients[i].user_name);
        }
    }
    pthread_mutex_unlock(&lock);

    // wrap in msg_t
    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_ONLINE_LIST;
    memcpy(msg.message, &list, sizeof(list));

    send(client_fd, &msg, sizeof(msg), 0);
}

void broadcast_notification(char *username, int status){

    // build notification message
    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_NOTIFICATION;
    
    if(status == ONLINE)
        snprintf(msg.message, sizeof(msg.message),
                "*** %s is ONLINE ***", username);
    else
        snprintf(msg.message, sizeof(msg.message),
                "*** %s is OFFLINE ***", username);

    // build updated online list
    onlinelist_t list;
    memset(&list, 0, sizeof(list));

    pthread_mutex_lock(&lock);

    // build current online list
    for(int i = 0; i < client_no; i++){
        if(clients[i].status == ONLINE){
            strcpy(list.user[list.count++],
                   clients[i].user_name);
        }
    }

    // send to all online clients except this user
    for(int i = 0; i < client_no; i++){
        if(clients[i].status == ONLINE &&
           strcmp(clients[i].user_name, username) != 0){

            // send notification
            send(clients[i].socket_fd,
                &msg, sizeof(msg), 0);

            // send updated list
            send(clients[i].socket_fd,
                &list, sizeof(list), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

void authentication(int client_fd){


    request_t req;
    char op = 0;

    while (1)
    {
        recv(client_fd, &req, sizeof(req), 0);
        op = 0;
        if (req.option == LOGIN)
        {

            pthread_mutex_lock(&lock);

            for (int i = 0; i < client_no; i++)
            {

                if (strcmp(clients[i].user_name, req.username) == 0)
                {

                    if ((strcmp(clients[i].password, req.password) == 0))
                    {
                        op = SUCCESS;
                        clients[i].status = ONLINE;
                        clients[i].socket_fd = client_fd;
                    }

                    else
                        op = WRONG_PASSWORD;

                    break;
                }
            }

            pthread_mutex_unlock(&lock);
            if (op != SUCCESS && op != WRONG_PASSWORD)
                op = USERNAME_NOT_FOUND;

            send(client_fd, &op, sizeof(op), 0);

            if (op == SUCCESS)
                break;
        }
        else if (req.option == SIGN_IN)
        {

            pthread_mutex_lock(&lock);
            for (int i = 0; i < client_no; i++)
            {

                if (strcmp(clients[i].user_name, req.username) == 0)
                {

                    op = USERNAME_FOUND;
                    break;
                }
            }
            if (op != USERNAME_FOUND)
            {

                strcpy(clients[client_no].user_name, req.username);
                strcpy(clients[client_no].password, req.password);
                clients[client_no].status = ONLINE;
                clients[client_no].socket_fd = client_fd;
                client_no++;
                op = SUCCESS;
            }

            pthread_mutex_unlock(&lock);

            send(client_fd, &op, sizeof(op), 0);
            if (op == SUCCESS)
                break;
        }
        else
        {
            op = FAILURE;
            send(client_fd, &op, sizeof(op), 0);
        }
    }
}

void message_handle(int client_fd,char *username){


    msg_t msg;

    while(1){

        memset(&msg, 0, sizeof(msg));
        int n=recv(client_fd,&msg,sizeof(msg),0);
        if(n <= 0){
            pthread_mutex_lock(&lock);
            for(int i = 0; i < client_no; i++){
                if(clients[i].socket_fd == client_fd){
                    clients[i].status = OFFLINE;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);
            broadcast_notification(username, OFFLINE);
            close(client_fd);
            return;
        }

        switch(msg.type){

            case MSG_SINGLE_CHAT:
                single_chat(&msg);
                break;
            case MSG_GROUP_CHAT:

                group_chat(&msg);
                break;

            case MSG_VIEW_USERS:
                send_online_users(client_fd);
                break;

            case MSG_LOGOUT:

                logout(username,client_fd);
                return;

        }

    }

}

void group_chat(msg_t * msg){

        pthread_mutex_lock(&lock);
    for(int i = 0; i < client_no; i++){
        if(clients[i].status == ONLINE &&
           strcmp(clients[i].user_name,msg->sender) != 0){

            // send notification
            send(clients[i].socket_fd,
                msg,sizeof(*msg), 0);

        }
    }

    pthread_mutex_unlock(&lock);

}

void single_chat(msg_t *msg){
    
    pthread_mutex_lock(&lock);

    for(int i=0;i<client_no;i++){

        if(strcmp(msg->receiver,clients[i].user_name)==0){

            send(clients[i].socket_fd,msg,sizeof(*msg), 0);
            break;
        }
            
    }

    pthread_mutex_unlock(&lock);

}

void logout(char *username,int client_fd){

    pthread_mutex_lock(&lock);

        for(int i = 0; i < client_no; i++){
            if(strcmp(clients[i].user_name,
                        username) == 0){
                clients[i].status = OFFLINE;
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        broadcast_notification(username, OFFLINE);
        close(client_fd);
        return;

}