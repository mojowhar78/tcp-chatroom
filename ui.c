#include "ui.h"

/*─────────────────────────────────────────
  TTYTalk - ui.c
  All window definitions live here ONLY
─────────────────────────────────────────*/

// window definitions
WINDOW *header_win;
WINDOW *chat_win;
WINDOW *users_win;
WINDOW *input_win;

// dimension definitions
int ROWS, COLS;
int CHAT_H, CHAT_W;
int USERS_W;
int INPUT_H;
int HEADER_H;

// ui mutex definition
pthread_mutex_t ui_mutex = PTHREAD_MUTEX_INITIALIZER;

/*─────────────────────────────────────────
  init_ui()
  Initialize ncurses and create all windows
─────────────────────────────────────────*/
void init_ui(){

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    curs_set(1);

    // get terminal size
    getmaxyx(stdscr, ROWS, COLS);

    // color pairs - dark theme
    init_pair(CLR_HEADER,  COLOR_CYAN,    COLOR_BLACK);
    init_pair(CLR_ONLINE,  COLOR_GREEN,   COLOR_BLACK);
    init_pair(CLR_MINE,    COLOR_YELLOW,  COLOR_BLACK);
    init_pair(CLR_OTHERS,  COLOR_WHITE,   COLOR_BLACK);
    init_pair(CLR_NOTIFY,  COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CLR_ERROR,   COLOR_RED,     COLOR_BLACK);
    init_pair(CLR_BORDER,  COLOR_CYAN,    COLOR_BLACK);
    init_pair(CLR_PRIVATE, COLOR_GREEN,   COLOR_BLACK);
    init_pair(CLR_TITLE,   COLOR_YELLOW,  COLOR_BLACK);

    // calculate dimensions
    HEADER_H = 7;
    INPUT_H  = 3;
    USERS_W  = 22;
    CHAT_W   = COLS - USERS_W;
    CHAT_H   = ROWS - HEADER_H - INPUT_H;

    // create windows
    header_win = newwin(HEADER_H, COLS,          0,                0);
    chat_win   = newwin(CHAT_H,   CHAT_W,         HEADER_H,         0);
    users_win  = newwin(CHAT_H,   USERS_W,        HEADER_H,         CHAT_W);
    input_win  = newwin(INPUT_H,  COLS,           ROWS - INPUT_H,   0);

    // enable scrolling in chat window
    scrollok(chat_win, TRUE);
    // keep cursor inside scrollable area
    wsetscrreg(chat_win, 1, CHAT_H - 2);

    draw_header();
    draw_borders();
}

/*─────────────────────────────────────────
  draw_header()
  TTYTalk ASCII art header - simple chars only
─────────────────────────────────────────*/
void draw_header(){

    wclear(header_win);

    // top border line
    wattron(header_win, COLOR_PAIR(CLR_BORDER) | A_BOLD);
    mvwhline(header_win, 0, 0, '=', COLS);
    wattroff(header_win, COLOR_PAIR(CLR_BORDER) | A_BOLD);

    // TTYTalk ASCII art - simple chars only
    wattron(header_win, COLOR_PAIR(CLR_HEADER) | A_BOLD);

    mvwprintw(header_win, 1, 4,
        " _____ _____ __  __  _____     _ _    ");
    mvwprintw(header_win, 2, 4,
        "|_   _|_   _|  \\/  |/  __ \\   | | |   ");
    mvwprintw(header_win, 3, 4,
        "  | |   | | | .  . || /  \\/_ _| | | __");
    mvwprintw(header_win, 4, 4,
        "  | |   | | | |\\/| || |   / _` | | |/ /");
    mvwprintw(header_win, 5, 4,
        "  |_|   |_| |_|  |_|\\_\\__/\\__,_|_|_|  \\");

    wattroff(header_win, COLOR_PAIR(CLR_HEADER) | A_BOLD);

    // tagline on right side
    wattron(header_win, COLOR_PAIR(CLR_TITLE));
    mvwprintw(header_win, 2, COLS - 28,
        "[ Terminal Chat System ]");
    mvwprintw(header_win, 3, COLS - 28,
        "[ type !exit to go back ]");
    wattroff(header_win, COLOR_PAIR(CLR_TITLE));

    // bottom border line
    wattron(header_win, COLOR_PAIR(CLR_BORDER) | A_BOLD);
    mvwhline(header_win, HEADER_H - 1, 0, '=', COLS);
    wattroff(header_win, COLOR_PAIR(CLR_BORDER) | A_BOLD);

    wrefresh(header_win);
}

/*─────────────────────────────────────────
  draw_borders()
  Draw borders for all windows
─────────────────────────────────────────*/
void draw_borders(){

    // chat window
    wattron(chat_win, COLOR_PAIR(CLR_BORDER));
    box(chat_win, '|', '-');
    mvwprintw(chat_win, 0, 2, "[ MESSAGES ]");
    wattroff(chat_win, COLOR_PAIR(CLR_BORDER));
    wrefresh(chat_win);

    // users window
    wattron(users_win, COLOR_PAIR(CLR_BORDER));
    box(users_win, '|', '-');
    mvwprintw(users_win, 0, 2, "[ ONLINE ]");
    wattroff(users_win, COLOR_PAIR(CLR_BORDER));
    wrefresh(users_win);

    // input window
    wattron(input_win, COLOR_PAIR(CLR_BORDER));
    box(input_win, '|', '-');
    mvwprintw(input_win, 0, 2, "[ MESSAGE ]");
    wattroff(input_win, COLOR_PAIR(CLR_BORDER));
    wrefresh(input_win);
}

/*─────────────────────────────────────────
  chat_print()
  Print message in chat window
  is_mine = 1 for your own messages
─────────────────────────────────────────*/
void chat_print(char *sender, char *msg, int is_mine){

    if(is_mine){
        wattron(chat_win, COLOR_PAIR(CLR_MINE) | A_BOLD);
        wprintw(chat_win, "  [you] ");
        wattroff(chat_win, A_BOLD);
        wprintw(chat_win, ": %s\n", msg);
        wattroff(chat_win, COLOR_PAIR(CLR_MINE));
    }
    else{
        wattron(chat_win, COLOR_PAIR(CLR_OTHERS) | A_BOLD);
        wprintw(chat_win, "  [%s] ", sender);
        wattroff(chat_win, A_BOLD);
        wprintw(chat_win, ": %s\n", msg);
        wattroff(chat_win, COLOR_PAIR(CLR_OTHERS));
    }
    wrefresh(chat_win);
}

/*─────────────────────────────────────────
  chat_print_private()
  Print private message in chat window
─────────────────────────────────────────*/
void chat_print_private(char *sender, char *msg){

    wattron(chat_win, COLOR_PAIR(CLR_PRIVATE) | A_BOLD);
    wprintw(chat_win, "  [private][%s] ", sender);
    wattroff(chat_win, A_BOLD);
    wprintw(chat_win, ": %s\n", msg);
    wattroff(chat_win, COLOR_PAIR(CLR_PRIVATE));
    wrefresh(chat_win);
}

/*─────────────────────────────────────────
  notify_print()
  Print notification/system message
─────────────────────────────────────────*/
void notify_print(char *msg){

    wattron(chat_win, COLOR_PAIR(CLR_NOTIFY) | A_BOLD);
    wprintw(chat_win, "  *** %s ***\n", msg);
    wattroff(chat_win, COLOR_PAIR(CLR_NOTIFY) | A_BOLD);
    wrefresh(chat_win);
}

/*─────────────────────────────────────────
  users_update()
  Refresh online users panel
─────────────────────────────────────────*/
void users_update(onlinelist_t *list){

    // clear and redraw
    werase(users_win);

    wattron(users_win, COLOR_PAIR(CLR_BORDER));
    box(users_win, '|', '-');
    mvwprintw(users_win, 0, 2, "[ ONLINE ]");
    wattroff(users_win, COLOR_PAIR(CLR_BORDER));

    // show count
    wattron(users_win, COLOR_PAIR(CLR_TITLE) | A_BOLD);
    mvwprintw(users_win, 1, 2, " Total: %d", list->count);
    wattroff(users_win, COLOR_PAIR(CLR_TITLE) | A_BOLD);

    // divider
    wattron(users_win, COLOR_PAIR(CLR_BORDER));
    mvwhline(users_win, 2, 1, '-', USERS_W - 2);
    wattroff(users_win, COLOR_PAIR(CLR_BORDER));

    // print each user
    for(int i = 0; i < list->count && i < CHAT_H - 5; i++){
        wattron(users_win, COLOR_PAIR(CLR_ONLINE));
        mvwprintw(users_win, i + 3, 2,
                  " (+) %-14s", list->user[i]);
        wattroff(users_win, COLOR_PAIR(CLR_ONLINE));
    }

    wrefresh(users_win);
}

/*─────────────────────────────────────────
  input_get()
  Get user input from input window
─────────────────────────────────────────*/
void input_get(char *buffer){

    // clear input line
    wmove(input_win, 1, 1);
    wclrtoeol(input_win);

    // fix right border after clrtoeol
    wattron(input_win, COLOR_PAIR(CLR_BORDER));
    mvwaddch(input_win, 1, COLS - 1, '|');
    wattroff(input_win, COLOR_PAIR(CLR_BORDER));

    // show prompt
    wattron(input_win, COLOR_PAIR(CLR_MINE) | A_BOLD);
    mvwprintw(input_win, 1, 2, "> ");
    wattroff(input_win, COLOR_PAIR(CLR_MINE) | A_BOLD);

    wrefresh(input_win);

    // get input with echo
    echo();
    curs_set(1);
    wgetnstr(input_win, buffer, 250);
    noecho();

    // clear after input
    wmove(input_win, 1, 1);
    wclrtoeol(input_win);
    wattron(input_win, COLOR_PAIR(CLR_BORDER));
    mvwaddch(input_win, 1, COLS - 1, '|');
    wattroff(input_win, COLOR_PAIR(CLR_BORDER));
    wrefresh(input_win);
}

/*─────────────────────────────────────────
  status_set()
  Update status bar in input window title
─────────────────────────────────────────*/
void status_set(char *msg){

    wattron(input_win, COLOR_PAIR(CLR_NOTIFY) | A_BOLD);
    mvwprintw(input_win, 0, 2, "[ %-18s]", msg);
    wattroff(input_win, COLOR_PAIR(CLR_NOTIFY) | A_BOLD);
    wrefresh(input_win);
}

/*─────────────────────────────────────────
  show_menu()
  Print chat menu in chat window
─────────────────────────────────────────*/
void show_menu(){

    wattron(chat_win, COLOR_PAIR(CLR_TITLE) | A_BOLD);
    wprintw(chat_win,
        "\n  +---------------------------+\n"
        "  |   1) Group Chat           |\n"
        "  |   2) Single Chat          |\n"
        "  |   3) View Online Users    |\n"
        "  |   4) Logout               |\n"
        "  +---------------------------+\n\n");
    wattroff(chat_win, COLOR_PAIR(CLR_TITLE) | A_BOLD);
    wrefresh(chat_win);
}

/*─────────────────────────────────────────
  destroy_ui()
  Cleanup all windows
─────────────────────────────────────────*/
void destroy_ui(){
    delwin(chat_win);
    delwin(users_win);
    delwin(input_win);
    delwin(header_win);
    endwin();
}