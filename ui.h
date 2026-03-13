#ifndef _UI_H_
#define _UI_H_

#include <ncurses.h>
#include "header.h"

// windows
extern WINDOW *header_win;
extern WINDOW *chat_win;
extern WINDOW *users_win;
extern WINDOW *input_win;

// dimensions
extern int ROWS, COLS;
extern int CHAT_H, CHAT_W;
extern int USERS_W;
extern int INPUT_H;
extern int HEADER_H;

// mutex for thread safe ui updates
extern pthread_mutex_t ui_mutex;

// color pairs
#define CLR_HEADER   1
#define CLR_ONLINE   2
#define CLR_MINE     3
#define CLR_OTHERS   4
#define CLR_NOTIFY   5
#define CLR_ERROR    6
#define CLR_BORDER   7
#define CLR_PRIVATE  8
#define CLR_TITLE    9

// function declarations
void init_ui();
void destroy_ui();
void draw_borders();
void draw_header();
void chat_print(char *sender, char *msg, int is_mine);
void chat_print_private(char *sender, char *msg);
void notify_print(char *msg);
void users_update(onlinelist_t *list);
void input_get(char *buffer);
void status_set(char *msg);
void show_menu();

#endif