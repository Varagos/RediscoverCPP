#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <vector>

using namespace std;

int MAX_CLIENTS = 10;

class RESP_PROTOCOL
{
  // A client sends an array of string, containing the command and its arguments
  // The server's reply type is command-specific
  /**
   * CRLF: Carriage Return and Line Feed (\r\n)
   * Basic Data Types:
   * - strings: These start with a "+" character, followed by the string value and a CRLF terminator.
   * - Errors: These start with a "-" character, followed by the error message and a CRLF terminator.
   * - Integers: These start with a ":" character, followed by the integer value and a CRLF terminator.
   * - Bulk strings: These start with a "$" character, followed by the length of the string, a CRLF terminator, the string value and a CRLF terminator.
   *    $<length>\r\n<data>\r\n
   * - Arrays: These start with a "*" character, followed by the number of elements in the array, a CRLF terminator, the elements of the array and a CRLF terminator.
   *
   * Handling Commands:
   * - A client sends the server an *array* consisting of only bulk strings.
   * - The server replies to clients, sending any valid RESP data type as a response.
   */
public:
  static string encode(string message)
  {
    return "+" + message + "\r\n";
  }
  static string decode(string message)
  {
    return message.substr(1, message.length() - 3);
  }

  static bool command_is_echo(string resp_msg)
  {
    return resp_msg == "ECHO";
  }
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

    const string command = tokens[2];

    // Example: Handling "PING" command
    if (tokens.size() >= 3 && (command == "PING" || command == "ping"))
    {
      return "+PONG\r\n";
    }

    // Example: Handling "ECHO" command with its message
    else if (tokens.size() >= 5 && (command == "ECHO" || command == "echo"))
    {
      // Assuming the ECHO message is the fifth token (after $<length> and the actual message)
      string echoMessage = tokens[4];
      return "$" + to_string(echoMessage.length()) + "\r\n" + echoMessage + "\r\n";
    }

    // Add more command handling as needed

    throw runtime_error("Unsupported command detected: " + msg);
  }
};

void handle_client(int client_fd)
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
      // Client may have forcefully closed the connection, without the normal TCP shutdown sequence
      cerr << "Failed to receive msg\n"
           << endl;
      break;
    }
    cout << "Received: " << buffer << "\n"
         << endl;

    // Using msg_length, create a new buffer with the exact size of the received message
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

int main(int argc, char **argv)
{
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  // domain, type, protocol
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    cerr << "Failed to create server socket\n";
    return 1;
  }
  //
  // // Since the tester restarts your program quite often, setting REUSE_PORT
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
  {
    cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);
  //
  if (::bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    cerr << "Failed to bind to port 6379\n";
    return 1;
  }
  //
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    cerr << "listen failed\n";
    return 1;
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
    cout << "Inserted while loop"
         << endl;
    // Blocks until a client connects to the server
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);

    if (client_fd < 0)
    {
      cerr << "Failed to accept client connection\n";
      return 1;
    }
    clients++;

    thread th1(handle_client, client_fd);
    th1.detach();
  }

  // close(server_fd);
  return 0;
}
