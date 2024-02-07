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
#include <unistd.h>
#include <vector>
#include "includes/Server.h"

using namespace std;

int main(int argc, char **argv)
{
  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  cout << "Logs from your program will appear here!\n";

  Server server = Server();
  server.run();

  return 0;
}
