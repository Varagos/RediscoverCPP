
#include "Server.h"
#include "CommandHandler.h"
#include "RespProtocol.h"
#include "InMemoryStorage.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

int MAX_CLIENTS = 10;

InMemoryStorage storage;

void Server::run()
{

    // domain, type, protocol
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        cerr << "Failed to create server socket\n";
        return;
    }
    //
    // // Since the tester restarts your program quite often, setting REUSE_PORT
    // // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
        0)
    {
        cerr << "setsockopt failed\n";
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    //
    if (::bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
        0)
    {
        cerr << "Failed to bind to port 6379\n";
        return;
    }
    //
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        cerr << "listen failed\n";
        return;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    // //
    cout << "Waiting for a client to connect...\n";

    vector<thread> threads;
    int max_clients = 10;
    // Accept many clients
    int clients = 0;
    while (clients < max_clients)
    {
        cout << "Inserted while loop" << endl;
        // Blocks until a client connects to the server
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);

        if (client_fd < 0)
        {
            cerr << "Failed to accept client connection\n";
            return;
        }
        clients++;

        // thread th1(handle_client, client_fd);
        thread th1(Server::start_client_thread, this, client_fd);

        th1.detach();
    }

    // close(server_fd);
}

void Server::start_client_thread(Server *serverInstance, int client_fd)
{
    serverInstance->handle_client(client_fd);
}

void Server::handle_client(int client_fd)
{
    int max_times = 1000;
    char response[] = "+PONG\r\n";
    // while client is connected
    while (1)
    {
        // keep alive

        char buffer[1024] = {0};
        size_t msg_length = recv(client_fd, buffer, 1024, 0);
        if (msg_length == 0)
        {
            cout << "Client disconnected\n"
                 << endl;
            break;
        }
        else if (msg_length < 0)
        {
            // Client may have forcefully closed the connection, without the normal
            // TCP shutdown sequence
            cerr << "Failed to receive msg\n"
                 << endl;
            break;
        }
        cout << "Received: " << buffer << "\n"
             << endl;

        // Using msg_length, create a new buffer with the exact size of the received
        // message
        char *msg = new char[msg_length + 1];
        strncpy(msg, buffer, msg_length);

        CommandHandler commandHandler(&storage);
        auto tokens = RespProtocol::tokenizeRESP(msg);
        string command = tokens[2];
        vector<string> args(tokens.begin() + 3, tokens.end());
        string resp_msg = commandHandler.handleCommand(command, args);
        // string resp_msg = RESP_PROTOCOL::parse_command_msg(msg);

        if (send(client_fd, resp_msg.c_str(), resp_msg.size(), 0) < 0)
        {
            cerr << "Failed to send PONG msg\n"
                 << endl;
            break;
        }
        cout << "Sent msg" << endl;
    }
    close(client_fd);
}