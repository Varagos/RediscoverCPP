
#include "Server.h"
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

struct ValueWithExpiry
{
    string value;
    chrono::steady_clock::time_point expiration_time;
};

map<string, ValueWithExpiry> storage;

class RESP_PROTOCOL
{
    // A client sends an array of string, containing the command and its arguments
    // The server's reply type is command-specific
    /**
     * CRLF: Carriage Return and Line Feed (\r\n)
     * Basic Data Types:
     * - strings: These start with a "+" character, followed by the string value
     * and a CRLF terminator.
     * - Errors: These start with a "-" character, followed by the error message
     * and a CRLF terminator.
     * - Integers: These start with a ":" character, followed by the integer value
     * and a CRLF terminator.
     * - Bulk strings: These start with a "$" character, followed by the length of
     * the string, a CRLF terminator, the string value and a CRLF terminator.
     *    $<length>\r\n<data>\r\n
     * - Arrays: These start with a "*" character, followed by the number of
     * elements in the array, a CRLF terminator, the elements of the array and a
     * CRLF terminator.
     *
     * Handling Commands:
     * - A client sends the server an *array* consisting of only bulk strings.
     * - The server replies to clients, sending any valid RESP data type as a
     * response.
     */
public:
    static string encode(string message) { return "+" + message + "\r\n"; }
    static string decode(string message)
    {
        return message.substr(1, message.length() - 3);
    }

    static bool command_is_echo(string resp_msg) { return resp_msg == "ECHO"; }
    static void printMessageWithVisibleNewlines(const char *msg)
    {
        cout << "Parsing command msg:";
        for (const char *p = msg; *p; ++p)
        {
            if (*p == '\r')
            {
                cout << "\\r";
            }
            else if (*p == '\n')
            {
                cout << "\\n";
            }
            else
            {
                cout << *p;
            }
        }
        cout << ":END" << endl;
    }

    static vector<string> tokenizeRESP(const string &msg)
    {
        vector<string> tokens;
        string token;
        for (char c : msg)
        {
            if (c == '\r')
            {
                continue;
            }
            if (c == '\n')
            {
                tokens.push_back(token);
                token = "";
            }
            else
            {
                token += c;
            }
        }
        return tokens;
    }

    static string parse_command_msg(const string &msg)
    {
        vector<string> tokens = tokenizeRESP(msg);

        // Ensure the message is correctly formatted
        if (tokens.empty() || tokens[0][0] != '*')
        {
            throw runtime_error("Invalid message format: " + msg);
        }
        if (tokens.size() < 3)
        {
            throw runtime_error("Invalid message format: " + msg);
        }

        // Convert the command to lowercase for easier comparison
        string command = tokens[2];
        for (char &c : command)
        {
            c = tolower(c);
        }

        // Example: Handling "PING" command
        if (tokens.size() >= 3 && (command == "ping"))
        {
            return "+PONG\r\n";
        }

        // Example: Handling "ECHO" command with its message
        else if (tokens.size() >= 5 && (command == "echo"))
        {
            // Assuming the ECHO message is the fifth token (after $<length> and the
            // actual message)
            string echoMessage = tokens[4];
            return "$" + to_string(echoMessage.length()) + "\r\n" + echoMessage +
                   "\r\n";
        }

        // SET command
        else if (tokens.size() >= 5 && (command == "set"))
        {
            string key = tokens[4];
            string value = tokens[6];
            if (tokens.size() < 9)
            {

                ValueWithExpiry valueObj;
                valueObj.value = value;
                storage[key] = valueObj;
                // return "OK"
                return "+OK\r\n";
            }

            string ex_command = tokens[8];
            if (ex_command == "EX" || ex_command == "ex")
            {
                int expiry = stoi(tokens[10]);
                // store the value with expiry
                ValueWithExpiry valueObj;
                valueObj.value = value;
                valueObj.expiration_time = chrono::steady_clock::now() +
                                           chrono::seconds(expiry);
                storage[key] = valueObj;
                return "+OK\r\n";
            }
            if (ex_command == "PX" || ex_command == "px")
            {
                int expiry = stoi(tokens[10]);
                // store the value with expiry
                ValueWithExpiry valueObj;
                valueObj.value = value;
                valueObj.expiration_time = chrono::steady_clock::now() +
                                           chrono::milliseconds(expiry);
                storage[key] = valueObj;
                return "+OK\r\n";
            }
        }
        // GET command
        else if (tokens.size() >= 3 && (command == "get"))
        {
            string key = tokens[4];
            ValueWithExpiry valueObj = storage[key];
            // return the value
            string value = valueObj.value;

            chrono::steady_clock::time_point expiration_time =
                valueObj.expiration_time;

            // If expiration time is not set, return the value
            if (expiration_time == chrono::steady_clock::time_point())
            {
                return "$" + to_string(value.length()) + "\r\n" + value + "\r\n";
            }

            // Passive expiration
            if (expiration_time < chrono::steady_clock::now())
            {
                storage.erase(key);
                // or $-1\r\n
                return "$-1\r\n";
            }

            return "$" + to_string(value.length()) + "\r\n" + value + "\r\n";
        }

        // Add more command handling as needed

        throw runtime_error("Unsupported command detected: " + msg);
    }
};

// void handle_client(int client_fd)
// {
//   int max_times = 1000;
//   char response[] = "+PONG\r\n";
//   // while client is connected
//   while (1)
//   {
//     // keep alive

//     char buffer[1024] = {0};
//     size_t msg_length = recv(client_fd, buffer, 1024, 0);
//     if (msg_length == 0)
//     {
//       cout << "Client disconnected\n"
//            << endl;
//       break;
//     }
//     else if (msg_length < 0)
//     {
//       // Client may have forcefully closed the connection, without the normal
//       // TCP shutdown sequence
//       cerr << "Failed to receive msg\n"
//            << endl;
//       break;
//     }
//     cout << "Received: " << buffer << "\n"
//          << endl;

//     // Using msg_length, create a new buffer with the exact size of the received
//     // message
//     char *msg = new char[msg_length + 1];
//     strncpy(msg, buffer, msg_length);

//     string resp_msg = RESP_PROTOCOL::parse_command_msg(msg);

//     if (send(client_fd, resp_msg.c_str(), resp_msg.size(), 0) < 0)
//     {
//       cerr << "Failed to send PONG msg\n"
//            << endl;
//       break;
//     }
//     cout << "Sent PONG msg\n"
//          << endl;
//   }
//   close(client_fd);
// }

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

        string resp_msg = RESP_PROTOCOL::parse_command_msg(msg);

        if (send(client_fd, resp_msg.c_str(), resp_msg.size(), 0) < 0)
        {
            cerr << "Failed to send PONG msg\n"
                 << endl;
            break;
        }
        cout << "Sent PONG msg\n"
             << endl;
    }
    close(client_fd);
}