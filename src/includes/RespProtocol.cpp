#include "RespProtocol.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string RespProtocol::encode(string message) { return "+" + message + "\r\n"; }
string RespProtocol::decode(string message)
{
    return message.substr(1, message.length() - 3);
}

void RespProtocol::printMessageWithVisibleNewlines(const char *msg)
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

vector<string> RespProtocol::tokenizeRESP(const string &msg)
{
    vector<string> tokens;
    string token;
    for (char c : msg)
    {
        if (c == '\r')
            continue;

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