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

void handle_client(int client_fd)
{
  int max_times = 1000;
  char response[] = "+PONG\r\n";
  // while client is connected
  while (max_times-- > 0)
  {
    // keep alive

    char buffer[1024] = {0};
    size_t msg_length = recv(client_fd, buffer, 1024, 0);
    cout << "Received: " << buffer << "\n";

    send(client_fd, response, strlen(response), 0);
    cout << "Sent PONG msg\n";
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
  while (max_clients-- > 0)
  {
    // Blocks until a client connects to the server
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
    cout << "Client connected\n";

    if (client_fd < 0)
    {
      cerr << "Failed to accept client connection\n";
      return 1;
    }

    thread th1(handle_client, client_fd);
    th1.detach();
  }

  close(server_fd);
  return 0;
}
