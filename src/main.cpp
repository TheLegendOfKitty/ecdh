#define NCURSES_NOMACROS

#include <cstdint>
//#include <ncurses.h>
#include <iostream>
#include <menu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/network.h"
#include <chrono>
#include <thread>
#include <ncurses.h>
#include <bits/types/time_t.h>
#include <time.h>
#include <panel.h>
#include "include/net_common.h"
#include <menu.h>
#include <signal.h>
#include <memory>
#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sys/stat.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


volatile bool cont = true;

void loadImageBlocking(const char* path){
    SDL_Event event;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Window *window = NULL;

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    window = SDL_CreateWindow("test", 0, 0, 500, 500, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);
    IMG_Init(IMG_INIT_PNG);
    texture = IMG_LoadTexture(renderer, path);
    SDL_Point size;
    SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
    SDL_SetWindowSize(window, size.x / 2, size.y / 2);
    while (1) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Event e;
        if ( SDL_PollEvent(&e) ) {
            if (e.type == SDL_QUIT)
                break;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                break;
        }
    }
    SDL_DestroyTexture(texture);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void signal_callback_handler(int signum){
   std::cout << "Caught Signal" << signum << std::endl;
   cont = false;
}

/* Read keyboard input into buffer */
int userInput(WINDOW* inputWin, packet* tx_pkt, int* ret, WINDOW* chatWin) {
   int i = 0;
   int ch;
   wmove(inputWin, 0, 0);
   wrefresh(inputWin);
   // Read 1 char at a time
   while ((ch = getch()) != '\n') {
      #ifdef DEBUG
      wprintw(chatWin, "%c", ch);
      wrefresh(chatWin);
      #endif
      // Backspace
      if (ch == 8 || ch == 127 || ch == KEY_LEFT || ch == 263) {
         if (i > 0) {
            wprintw(inputWin, "\b \b\0");
            tx_pkt->buf[--i] = '\0';
            wrefresh(inputWin);
         }
         else {
            wprintw(inputWin, "\b \0");
         }
      }
      else if (ch == KEY_RESIZE) {
        continue;
      }
      else if(ch == 27){
        *ret = ESCAPE_RET;
        return 0;
      }
      else if(ch < 32 || ch > 126){
         continue;
      }
      // Otherwise put in buffer
      else if (ch != ERR) {
         if (i < BUFFERSIZE - 1) {
            strcat(tx_pkt->buf, (char *)&ch);
            i++;
            wprintw(inputWin, (char *)&ch);
            wrefresh(inputWin);
         }
         // Unless buffer is full
         else {
            wprintw(inputWin, "\b%s", (char *)&ch);
            tx_pkt->buf[(i - 1)] = '\0';
            strcat(tx_pkt->buf, (char *)&ch);
            wrefresh(inputWin);
         }
      }
   }
   // Null terminate, clear input window
   tx_pkt->buf[i] = '\0';
   wclear(inputWin);
   wrefresh(inputWin);
   *ret = ENTER_RET;
   return i;
}


class Client : public olc::net::client_interface<MessageType>{
    	private:
	std::chrono::time_point<std::chrono::system_clock> lastFetch = std::chrono::system_clock::now();		

	public:
        int message(std::string toSend){
            olc::net::message<MessageType> msg;
            msg.header.id = MessageType::ClientMessage;
            if(toSend.length() > 128){
                return 1;
            }
            char chSend[128];

	    strcpy(chSend, toSend.c_str());

            msg << chSend;
            Send(msg);

            return 0;
        }
	/*std::array<64, 128> newMessages(){
	   olc::net::message<MessageType> msg;
	   msg.header.id = MessageType::ClientMessageUpdate;
	   
	   msg << lastFetch;

	   this->lastFetch = std::chrono::system_clock::now();
	   Send(msg);
	}*/
};

/*int main(int argc, char** argv){
    if(!(argc >= 2)){
        std::cout << "[ERROR] Enter the string to send!" << std::endl;
        return 1;
    }
    Client c;
    c.Connect("127.0.0.1", 60000);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    if(c.message(std::string(argv[2]))){
        std::cout << "[ERROR] String Length Is Longer Than 128" << std::endl;
    }

    while(true){
        
    }
}
*/


long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

void fillVecWithFiles(std::vector<std::string> &vec, std::string path){
    for (const auto & entry : std::filesystem::directory_iterator(path)){
        vec.push_back(entry.path());  
    }
}

int main(int argc, char** argv){
   signal(SIGINT, signal_callback_handler);
  WINDOW *my_wins[4];
	PANEL  *my_panels[4];
	PANEL  *top;
	int ch;
  char str[80];
  int row,col;
   char users[1024][16] = {
     "test user 1",
     "test user 2",
     '\0'
  };
  ITEM** user_items;
  int user_c;
  MENU* user_menu;
  int n_choices, i;
  WINDOW* user_window;
  bool userWinUp = false;
  bool draw = true;
  ITEM *cur_item;

	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
   init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
   std::vector<ITEM*> vec_items;
   std::vector<std::string> files;
    std::string path = argv[1];
    
    fillVecWithFiles(files, path);
    n_choices = std::distance(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator());
    for(i = 0; i < n_choices && i<512; ++i){
            vec_items.push_back(new_item(files[i].c_str(),""));
    }

   //n_choices = ARRAY_SIZE(users);
   /*user_items = (ITEM **) calloc(n_choices, sizeof(ITEM*));
   for(i = 0; i < n_choices; ++i){
      user_items[i] = new_item(users[i], users[i]);
   }*/
   for(i = 0; i<vec_items.size(); i++){
        if(vec_items[i] != nullptr){
            std::cout << vec_items[i]->name.str << std::endl;
            continue;
        }

        //std::cout << i << "is nullptr!" << std::endl;
        vec_items.erase(vec_items.begin() + i);
   }
   user_items = vec_items.data();
   user_items[vec_items.size() + 1] = NULL;

   user_menu = new_menu((ITEM **) user_items);
   menu_opts_off(user_menu, O_SHOWDESC);

   user_window = newwin(20, 120, 4, 4);
   keypad(user_window, TRUE);

   set_menu_win(user_menu, user_window);
   set_menu_sub(user_menu, derwin(user_window, 15, 110, 3, 1));
   set_menu_format(user_menu, 13, 2);
   set_menu_mark(user_menu, " *");

   box(user_window, 0, 0);

   post_menu(user_menu); 
   

  WINDOW* inputWinBox;

  inputWinBox = subwin(stdscr, (LINES * 0.2) - 1, COLS, (LINES * 0.8) + 1, 0);
  box(inputWinBox, 0, 0);
  WINDOW* inputWin = subwin(inputWinBox, (LINES * 0.2) - 3, COLS - 2, (LINES * 0.8) + 2, 1);

  packet tx_pkt;
  packet *tx_pkt_ptr = &tx_pkt;
  int bufSize;


  WINDOW* chatWinBox = subwin(stdscr, (LINES * 0.8), COLS, 0, 0);
   box(chatWinBox, 0, 0);
   
   mvwaddch(chatWinBox, 0, (COLS * 0.5) - 6, ACS_RTEE);
   wattron(chatWinBox, COLOR_PAIR(3));
   mvwaddstr(chatWinBox, 0, (COLS * 0.5) - 5, " Messages " );
   wattroff(chatWinBox, COLOR_PAIR(3));
   mvwaddch(chatWinBox, 0, (COLS * 0.5) + 4, ACS_LTEE );
   wrefresh(chatWinBox);

  WINDOW* chatWin = subwin(chatWinBox, (LINES * 0.8 - 2), COLS - 2, 1, 1);

   scrollok(chatWin, TRUE);
   doupdate();
  
   Client c;
   c.Connect("127.0.0.1", 60000);
   draw = true;
   std::thread updateMsgs([&](){
      while(true){
      while(!c.Incoming().empty()){
      auto msg = c.Incoming().pop_front().msg;
      //#define DEBUG
      #ifdef DEBUG
      wprintw(chatWin, "[DEBUG] Incoming packet: ");
      switch(msg.header.id){
         case MessageType::ServerAccept:
            wprintw(chatWin, "Server Accept\n");
            break;
         case MessageType::ClientMessage:
            wprintw(chatWin, "Client Message\n");
            break;
         case MessageType::ClientMessageUpdate:
            wprintw(chatWin, "Client Message Recieved\n");
            break;
         case MessageType::ServerMessage:
            wprintw(chatWin, "Server Message\n");
            break;
         case MessageType::ServerPing:
            wprintw(chatWin, "Server Ping\n");
            break;
         case MessageType::ServerDeny:
            wprintw(chatWin, "Server Deny\n");
            break;
         case MessageType::UserUpdate:
            wprintw(chatWin, "User Update\n");
            break;
         default:
            wprintw(chatWin, "unable to determine packet\n");
            break;
      }
      #endif

      //wprintw(chatWin, "%c", msg.header.id);
      
      if(msg.header.id == MessageType::ClientMessageUpdate){
         #ifdef DEBUG
         wprintw(chatWin, "[DEBUG] Incoming Message\n");
         #endif
         char pkt[BUFFERSIZE];
         msg >> pkt;
         if(draw){
         wprintw(chatWin, "%s\n", pkt);
          wrefresh(chatWin);
         update_panels();
    
		   doupdate();
         wcursyncup(inputWin);
         wrefresh(inputWin);
         refresh();
         }
	   }/*
      if(msg.header.id == MessageType::UserUpdate){
         #ifdef DEBUG
         wprintw(chatWin, "[DEBUG] User Update\n");
         #endif
         msg >> users;
         n_choices = ARRAY_SIZE(users);
         free(user_items);
        user_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
        for(i = 0; i < n_choices; ++i)
                user_items[i] = new_item(users[i], users[i]);
      }*/
    }}
   });
   std::thread run([&](){
      while(c.IsConnected()){
      draw = true;
    wcursyncup(inputWin);
    wrefresh(inputWin);
    refresh();
    // Wipe packet space
    memset(&tx_pkt, 0, sizeof(packet));
    // Set packet options as untouched
    tx_pkt.options = INVALID;
    // Read user kb input, return number of chars
    int ret;
    bufSize = userInput(inputWin, tx_pkt_ptr, &ret, chatWin);
    if(bufSize > BUFFERSIZE){
       continue;
    }
    if(ret == ESCAPE_RET){
      mvprintw(LINES - 2, 0, "ESCAPE HIT!");
      continue;
    }
    if(tx_pkt.buf[0] == ':'){
      if(!strncmp(tx_pkt.buf, ":help", sizeof(":help"))){
         wprintw(chatWin, "help cmd");
      }
      else if(!strncmp(tx_pkt.buf, ":users", sizeof(":users"))){
         olc::net::message<MessageType> userReq;
         userReq.header.id = MessageType::UserRequest;
         c.Send(userReq);
         wprintw(chatWin, "%s", users[0]);
         wprintw(chatWin, "%s", users[1]);
         wrefresh(chatWin);
         update_panels();
         doupdate();
         userWinUp = true;
         draw = false;
            redrawwin(user_window);
            wrefresh(user_window);
            [&]{
             while((ch = wgetch(user_window)) /* ESCAPE */)
               {       
                  switch(ch)
                     {	case KEY_DOWN:
                        menu_driver(user_menu, REQ_DOWN_ITEM);
                        break;
                     case KEY_UP:
                        menu_driver(user_menu, REQ_UP_ITEM);
                        break;
                     case KEY_LEFT:
                        menu_driver(user_menu, REQ_LEFT_ITEM);
                        break;
                     case KEY_RIGHT:
                        menu_driver(user_menu, REQ_RIGHT_ITEM);
                        break;
                     case KEY_NPAGE:
                        menu_driver(user_menu, REQ_SCR_DPAGE);
                        break;
                     case KEY_PPAGE:
                        menu_driver(user_menu, REQ_SCR_UPAGE);
                        break;
                     case 10 /* enter */:{
                        cur_item = current_item(user_menu);
                        /*mvprintw(LINES - 2, 0, "You have chosen %d item with name %s and description %s", 
                        item_index(cur_item) + 1,  item_name(cur_item), 
                        item_description(cur_item));*/
                        std::string currentExtension = std::filesystem::path(item_name(cur_item)).extension();
                        /*for(std::string extension : picExtensions){
                           if(currentExtension == extension){
                                    
                                 loadImageBlocking(item_name(cur_item));
                                 break;
                           }
                        }*/
                        olc::net::message<MessageType> fileMsg;
                                 fileMsg.header.id = MessageType::FileSend;
                                 std::ifstream file(
                                    std::filesystem::path(item_name(cur_item)),
                                    std::ios::binary | std::ios::ate
                                 );
                                 
                                 size_t size = std::filesystem::file_size(item_name(cur_item));
                                 /*wprintw(chatWin, "%zu", size);
                                 wrefresh(chatWin);*/
                                 //long size = GetFileSize(item_name(cur_item));
                                 file.seekg(0, std::ios::beg);

                                 std::vector<char> buffer(size);
                                 if (file.read(buffer.data(), size))
                                 {
                                    char x[fSize];
                                    strncpy(x, buffer.data(), fSize);
                                    //x[fSize - 1] = '\0';
                                    fileMsg << x;
                                    c.Send(fileMsg);
                                 }
                        refresh();
                        pos_menu_cursor(user_menu);
                        break;
                     }
                     case 27 /* ESCAPE */:
                        userWinUp = false;
                        draw = true;
                        return;
                     }
                     
                wrefresh(user_window);
	            }	
               }();
               redrawwin(chatWin);
               redrawwin(inputWin);
         }
      
      wrefresh(chatWin);
      update_panels();
      doupdate();
      continue;
    }
    olc::net::message<MessageType> msg;
    msg.header.id = MessageType::ClientMessage;
    msg << tx_pkt.buf;
    c.Send(msg);
    //wprintw(chatWin, "%s\n", tx_pkt.buf);
    
    wrefresh(chatWin);
   
    //wprintw(chatWinBox, "%s", tx_pkt.buf);
    // Set default send flag to True
    //send_flag = 1;

     /*switch(ch){
	      case 9:
          mvprintw(10, 0, "%s", tx_pkt.buf);	   
     }*/
     //mvprintw(LINES - 2, 0, "%s", tx_pkt.buf);
    //mvprintw(LINES - 2, 0, "%d", ch);
		update_panels();
    
		doupdate();}
   });
  while(c.IsConnected() && cont){
     
  }
   olc::net::message<MessageType> disc;
   disc.header.id = MessageType::ClientDisconnecting;
   c.Send(disc);
  c.Disconnect();
  updateMsgs.detach();
  run.detach();
  endwin();
  return 0;

}
