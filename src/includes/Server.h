// Server.h
#ifndef SERVER_H
#define SERVER_H

#include <thread> // For std::thread
#include <string> // For std::string

class Server
{

public:
    Server() {}
    Server(int port) : port(port) {}
    int port = 6379;

    void run();

private:
    int server_fd;

    void static start_client_thread(Server *server, int client_fd);

    void handle_client(int client_fd);
};
#endif