// test_ui.c
#include "header.h"
#include "ui.h"

int main(){
    
    // initialize ui
    init_ui();
    
    // test online users
    onlinelist_t list;
    memset(&list, 0, sizeof(list));
    strcpy(list.user[0], "user1");
    strcpy(list.user[1], "user2");
    strcpy(list.user[2], "user3");
    list.count = 3;
    users_update(&list);
    
    // test notifications
    notify_print("user1 is ONLINE");
    notify_print("user2 is ONLINE");
    
    // test chat messages
    chat_print("user1", "hello everyone!", 0);
    chat_print("user2", "hi there!", 0);
    chat_print("me", "wassup!", 1);
    chat_print("user1", "how are you?", 0);
    
    // test status bar
    status_set("GROUP CHAT");
    
    // test input
    char buffer[256];
    input_get(buffer);
    
    // print what was typed
    chat_print("you", buffer, 1);
    
    // wait for one more input to exit
    input_get(buffer);
    
    destroy_ui();
    return 0;
}