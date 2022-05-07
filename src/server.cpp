#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rwops.h>
#include <asio/buffer.hpp>
#include <asio/registered_buffer.hpp>
#include <cstdint>
#include <iostream>
#include <ostream>

#include "include/network.h"
#include "include/net_common.h"
#include "include/stacktrace.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void loadImageVoid(void* data, int size){
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    SDL_RWops* img = SDL_RWFromConstMem(data, size);
    SDL_Surface* surface = IMG_Load_RW(img, 1);
    SDL_Window* window = SDL_CreateWindow("test", 0, 0, 500, 500, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    IMG_Init(IMG_INIT_PNG);
    SDL_Point imgSize;
    SDL_SetWindowSize(window, surface->w / 2, surface->h / 2);
    while (1) {
        SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, surface), NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Event e;
        if ( SDL_PollEvent(&e) ) {
            if (e.type == SDL_QUIT)
                break;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                break;
        }
    }
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

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

class Server : public olc::net::server_interface<MessageType>{
    public:
        Server(uint16_t nPort) : olc::net::server_interface<MessageType>(nPort){}
    protected:
        virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<MessageType>> client)
	    {
            olc::net::message<MessageType> msg;
            
            msg.header.id = MessageType::ServerAccept;
            client->Send(msg);
            /*olc::net::message<MessageType> userListNew;
            userListNew.header.id = MessageType::UserUpdate;
            char users[1024][16];
            for(int i = 0; i < 1024 && i < m_deqConnections.size(); i++){
                if(!m_deqConnections[i]->err){
                    std::sprintf(users[i], "%d", m_deqConnections[i]->GetID());
                    std::cout << "Sending User: " << users[i] << std::endl;
                }
                
               
                //users[i] = m_deqConnections[i]->GetID()
            }
            for(int i = m_deqConnections.size() + 1; i < 1024; i++){
                std::strcpy(users[i], "");
            }
             std::sprintf(users[m_deqConnections.size()], "%d", nIDCounter);
            std::cout << "Size: " << m_deqConnections.size() << std::endl;
            std::cout << "ID COUNT:" << nIDCounter << std::endl;
            userListNew << users;
             for(int i = 0; i < m_deqConnections.size(); i++){
                    //std::cout << m_deqConnections[i]->GetID() << std::endl;
                    m_deqConnections[i]->Send(userListNew);
            }
            client->Send(userListNew);*/
            return true;
	    }

        // Called when a client appears to have disconnected
        virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<MessageType>> client)
        {
            std::cout << "Removing client [" << client->GetID() << "]\n";
            /*olc::net::message<MessageType> userListNew;
            userListNew.header.id = MessageType::UserUpdate;
            char users[1024][16];
            for(int i = 0; i < 1024 && i < m_deqConnections.size(); i++){
                if(m_deqConnections[i] == client){
                        continue;
                }
                std::sprintf(users[i], "%d", m_deqConnections[i]->GetID());
                std::cout << users[i] << std::endl;
                //users[i] = m_deqConnections[i]->GetID()
            }
            std::cout << m_deqConnections.size();
            for(int i = m_deqConnections.size() + 1; i < 1024; i++){
                std::strcpy(users[i], "");
            }
            userListNew << users;
             for(int i = 0; i < m_deqConnections.size(); i++){
                    //std::cout << m_deqConnections[i]->GetID() << std::endl;
                    
                    m_deqConnections[i]->Send(userListNew);
            }*/
        }

        // Called when a message arrives
        virtual void OnMessage(std::shared_ptr<olc::net::connection<MessageType>> client, olc::net::message<MessageType>& msg)
        {
            olc::net::message<MessageType> ping;
            ping.header.id = MessageType::ServerPing;
            MessageAllClients(ping);
            switch (msg.header.id)
            {
            case MessageType::ServerPing:
            {
                std::cout << "[" << client->GetID() << "]: Server Ping\n";

                // Simply bounce message back to client
                client->Send(msg);
            }
            break;

            case MessageType::ClientMessage:
            {
                char pkt[BUFFERSIZE];
                try{
                    /*char recv[128];
                msg >> recv;*/
                
                msg >> pkt;
                std::cout << "[" << client->GetID() << "]: Client Message\n";
                }
                catch(std::exception& e){
                    std::cout << Backtrace() << std::endl;
                    std::cout << e.what() << std::endl;
                    throw(e);
                }
                std::cout << pkt << std::endl;
		
                olc::net::message<MessageType> toSend;
                toSend.header.id = MessageType::ClientMessageUpdate;
                toSend << pkt;
                std::cout << m_deqConnections.size() << std::endl;
                for(int i = 0; i < m_deqConnections.size(); i++){
                    //std::cout << m_deqConnections[i]->GetID() << std::endl;
                    m_deqConnections[i]->Send(toSend);
                }
		        //MessageAllClients(msg, client);
                
                
                // Construct a new message and send it to all clients
                /*
                olc::net::message<MessageType> msg;
                msg.header.id = MessageType::ServerMessage;
                msg << client->GetID();
                MessageAllClients(msg, client);
                */

            }
            case MessageType::UserRequest:
                break;
            
            case MessageType::ClientDisconnecting:
                OnClientDisconnect(client);

                // Off you go now, bye bye!
                client.reset();

                // Then physically remove it from the container
                m_deqConnections.erase(
                    std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());

            case MessageType::FileSend:
                char buf[fSize];
                msg >> buf;
                std::cout << "[" << client->GetID() << "]: Client File\n";
                /*for(int i = 0; i < size; i++){
                    std::cout << buf[i];
                }*/
                for(char t : buf){
                    std::cout << t;
                }
            default:
                break;
            
            break;
            }
        }

};

int main(int argc, char** argv)
{
	Server server(60000); 
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}
	


	return 0;
}
