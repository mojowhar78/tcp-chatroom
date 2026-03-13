#include "header.h"
#include "ui.h"

/*─────────────────────────────────────────
  TTYTalk - client.c
─────────────────────────────────────────*/

// global username
char my_username[30];

// function declarations
void authentication(int client_fd);
void print_online_users(int client_fd);
void *receive_handler(void *arg);
void handle_chat(int client_fd);

/*─────────────────────────────────────────
  main()
─────────────────────────────────────────*/
int main(){

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_info;
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family      = AF_INET;
    server_info.sin_port        = htons(SERVER_PORT);
    server_info.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(connect(client_fd, (struct sockaddr*)&server_info,
               sizeof(server_info)) < 0){
        perror("Connect");
        exit(1);
    }

    // auth happens BEFORE ncurses
    // so errors print normally to terminal
    printf("\n  Welcome to TTYTalk!\n");
    printf("  ==================\n\n");

    authentication(client_fd);

    // start ncurses UI after auth
    init_ui();

    // recv and display online users
    print_online_users(client_fd);

    // notify "you are now connected"
    pthread_mutex_lock(&ui_mutex);
    notify_print("You are now connected!");
    status_set("LOBBY");
    pthread_mutex_unlock(&ui_mutex);

    // start receiver thread
    int *p = malloc(sizeof(int));
    *p = client_fd;
    pthread_t recv_tid;
    pthread_create(&recv_tid, NULL, receive_handler, p);
    pthread_detach(recv_tid);

    // main thread handles chat
    handle_chat(client_fd);

    destroy_ui();
    return 0;
}

/*─────────────────────────────────────────
  authentication()
  Handles register/login before UI starts
─────────────────────────────────────────*/
void authentication(int client_fd){

    char username[30];
    char password[10];
    request_t req;
    char result = 0;
    int option;

    while(1){
        result = 0;

        printf("  1) Register\n");
        printf("  2) Login\n");
        printf("  Option: ");
        scanf("%d", &option);

        printf("  Username: ");
        scanf("%s", username);
        printf("  Password: ");
        scanf("%s", password);

        strcpy(req.username, username);
        strcpy(req.password, password);
        req.option = option;

        send(client_fd, &req, sizeof(req), 0);

        switch(option){
            case SIGN_IN:
                recv(client_fd, &result, sizeof(result), 0);
                if(result == SUCCESS)
                    printf("  [OK] Registration successful!\n\n");
                else if(result == USERNAME_FOUND)
                    printf("  [!] Username already exists!\n\n");
                break;

            case LOGIN:
                recv(client_fd, &result, sizeof(result), 0);
                if(result == SUCCESS)
                    printf("  [OK] Login successful!\n\n");
                else if(result == USERNAME_NOT_FOUND)
                    printf("  [!] Username not found!\n\n");
                else if(result == WRONG_PASSWORD)
                    printf("  [!] Wrong password!\n\n");
                break;

            default:
                printf("  [!] Invalid option!\n\n");
                break;
        }

        if(result == SUCCESS){
            strcpy(my_username, username);
            return;
        }
    }
}

/*─────────────────────────────────────────
  print_online_users()
  Recv and display initial online users list
─────────────────────────────────────────*/
void print_online_users(int client_fd){

    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    recv(client_fd, &msg, sizeof(msg), 0);

    onlinelist_t list;
    memset(&list, 0, sizeof(list));
    memcpy(&list, msg.message, sizeof(list));

    pthread_mutex_lock(&ui_mutex);
    users_update(&list);
    pthread_mutex_unlock(&ui_mutex);
}

/*─────────────────────────────────────────
  receive_handler()
  Receiver thread - handles all incoming data
─────────────────────────────────────────*/
void *receive_handler(void *arg){

    int client_fd = *(int*)arg;
    free(arg);

    msg_t msg;
    onlinelist_t list;

    while(1){

        memset(&msg, 0, sizeof(msg));
        int n = recv(client_fd, &msg, sizeof(msg), 0);

        // server disconnected
        if(n <= 0){
            pthread_mutex_lock(&ui_mutex);
            notify_print("Disconnected from server!");
            pthread_mutex_unlock(&ui_mutex);
            exit(0);
        }

        pthread_mutex_lock(&ui_mutex);

        switch(msg.type){

            case MSG_NOTIFICATION:
                // print notification
                notify_print(msg.message);

                // recv updated online list
                memset(&list, 0, sizeof(list));
                recv(client_fd, &list, sizeof(list), 0);

                // update users panel
                users_update(&list);
                break;

            case MSG_GROUP_CHAT:
                chat_print(msg.sender, msg.message, 0);
                break;

            case MSG_SINGLE_CHAT:
                chat_print_private(msg.sender, msg.message);
                break;

            case MSG_ONLINE_LIST:
                memset(&list, 0, sizeof(list));
                memcpy(&list, msg.message, sizeof(list));
                users_update(&list);
                break;
        }

        pthread_mutex_unlock(&ui_mutex);
    }
    return NULL;
}

/*─────────────────────────────────────────
  handle_chat()
  Main chat loop - handles user input
─────────────────────────────────────────*/
void handle_chat(int client_fd){

    msg_t msg;
    char buffer[256];
    char choice_str[10];

    while(1){

        // show menu
        pthread_mutex_lock(&ui_mutex);
        show_menu();
        status_set("CHOOSE OPTION");
        pthread_mutex_unlock(&ui_mutex);

        // get choice
        pthread_mutex_lock(&ui_mutex);
        input_get(choice_str);
        pthread_mutex_unlock(&ui_mutex);

        int choice = atoi(choice_str);

        memset(&msg, 0, sizeof(msg));
        strcpy(msg.sender, my_username);

        switch(choice){

            /*─── GROUP CHAT ───*/
            case 1:
                pthread_mutex_lock(&ui_mutex);
                notify_print("--- Entered Group Chat ---");
                status_set("GROUP CHAT");
                pthread_mutex_unlock(&ui_mutex);

                while(1){
                    pthread_mutex_lock(&ui_mutex);
                    input_get(buffer);
                    pthread_mutex_unlock(&ui_mutex);

                    // exit group chat
                    if(strcmp(buffer, "!exit") == 0){
                        pthread_mutex_lock(&ui_mutex);
                        notify_print("--- Left Group Chat ---");
                        pthread_mutex_unlock(&ui_mutex);
                        break;
                    }

                    // send message
                    msg.type = MSG_GROUP_CHAT;
                    strcpy(msg.message, buffer);
                    send(client_fd, &msg, sizeof(msg), 0);

                    // show your own message
                    pthread_mutex_lock(&ui_mutex);
                    chat_print(my_username, buffer, 1);
                    pthread_mutex_unlock(&ui_mutex);

                    memset(msg.message, 0, sizeof(msg.message));
                }
                break;

            /*─── SINGLE CHAT ───*/
            case 2:
                pthread_mutex_lock(&ui_mutex);
                notify_print("--- Single Chat: Enter receiver name ---");
                status_set("SINGLE CHAT");
                input_get(msg.receiver);
                pthread_mutex_unlock(&ui_mutex);

                // check not messaging yourself
                if(strcmp(msg.receiver, my_username) == 0){
                    pthread_mutex_lock(&ui_mutex);
                    notify_print("[!] Cannot message yourself!");
                    pthread_mutex_unlock(&ui_mutex);
                    break;
                }

                pthread_mutex_lock(&ui_mutex);
                notify_print("--- Type !exit to go back ---");
                pthread_mutex_unlock(&ui_mutex);

                while(1){
                    pthread_mutex_lock(&ui_mutex);
                    input_get(buffer);
                    pthread_mutex_unlock(&ui_mutex);

                    // exit single chat
                    if(strcmp(buffer, "!exit") == 0){
                        pthread_mutex_lock(&ui_mutex);
                        notify_print("--- Left Single Chat ---");
                        pthread_mutex_unlock(&ui_mutex);
                        break;
                    }

                    // send message
                    msg.type = MSG_SINGLE_CHAT;
                    strcpy(msg.message, buffer);
                    send(client_fd, &msg, sizeof(msg), 0);

                    // show your own message
                    pthread_mutex_lock(&ui_mutex);
                    chat_print_private(my_username, buffer);
                    pthread_mutex_unlock(&ui_mutex);

                    memset(msg.message, 0, sizeof(msg.message));
                }
                break;

            /*─── VIEW USERS ───*/
            case 3:
                msg.type = MSG_VIEW_USERS;
                send(client_fd, &msg, sizeof(msg), 0);
                // receiver thread handles response
                break;

            /*─── LOGOUT ───*/
            case 4:
                msg.type = MSG_LOGOUT;
                send(client_fd, &msg, sizeof(msg), 0);

                pthread_mutex_lock(&ui_mutex);
                notify_print("Goodbye! Logging out...");
                pthread_mutex_unlock(&ui_mutex);

                sleep(1);
                destroy_ui();
                exit(0);

            default:
                pthread_mutex_lock(&ui_mutex);
                notify_print("[!] Invalid option!");
                pthread_mutex_unlock(&ui_mutex);
                break;
        }
    }
}