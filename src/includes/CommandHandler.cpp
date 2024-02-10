#include "CommandHandler.h"
#include "IKeyValueStorage.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

CommandHandler::CommandHandler(IKeyValueStorage *storage) : storage(storage) {}

string CommandHandler::handleCommand(const string &cmd, vector<string> &args)
{
    string command = cmd;
    // Convert the command to lowercase for easier comparison
    for (char &c : command)
    {
        c = tolower(c);
    }

    // Example: Handling "PING" command
    if (command == "ping")
    {
        return handlePing(args);
    }

    // Example: Handling "ECHO" command with its message
    else if (command == "echo")
    {
        return handleEcho(args);
    }

    // SET command
    else if (command == "set")
    {
        return handleSet(args);
    }
    // GET command
    else if (command == "get")
    {
        return handleGet(args);
    }

    throw runtime_error("Unsupported command detected: " + command);
}

string CommandHandler::handlePing(const vector<string> &args)
{
    return "+PONG\r\n";
}

string CommandHandler::handleEcho(const vector<string> &args)
{
    // Assuming the ECHO message is the fifth token (after $<length> and the
    // actual message)
    string echoMessage = args[1]; // tokens[4];
    return "$" + to_string(echoMessage.length()) + "\r\n" + echoMessage +
           "\r\n";
}

string CommandHandler::handleSet(const vector<string> &args)
{
    string key = args[1];   // tokens[4];
    string value = args[3]; // tokens[6];
    // if (tokens.size() < 9)
    if (args.size() < 6)
    {

        storage->set(key, value, nullopt);
        // ValueWithExpiry valueObj;
        // valueObj.value = value;
        // storage[key] = valueObj;
        // return "OK"
        return "+OK\r\n";
    }

    // string ex_command = tokens[8];
    string ex_command = args[5];
    if (ex_command == "EX" || ex_command == "ex")
    {
        // int expiry = stoi(tokens[10]);
        int expiry = stoi(args[7]);
        // store the value with expiry
        ValueWithExpiry valueObj;
        valueObj.value = value;
        valueObj.expiration_time = chrono::steady_clock::now() +
                                   chrono::seconds(expiry);
        storage->set(key, value, chrono::seconds(expiry));
        // storage[key] = valueObj;
        return "+OK\r\n";
    }
    if (ex_command == "PX" || ex_command == "px")
    {
        // int expiry = stoi(tokens[10]);
        int expiry = stoi(args[7]);
        // store the value with expiry
        ValueWithExpiry valueObj;
        valueObj.value = value;
        valueObj.expiration_time = chrono::steady_clock::now() +
                                   chrono::milliseconds(expiry);
        // storage[key] = valueObj;
        storage->set(key, value, chrono::milliseconds(expiry));
        return "+OK\r\n";
    }
}

string CommandHandler::handleGet(const vector<string> &args)
{
    // string key = tokens[4];
    string key = args[1];
    // ValueWithExpiry valueObj = storage[key];
    // return the value
    // string value = valueObj.value;

    string value = storage->get(key).value_or("");
    if (value == "")
    {
        return "$-1\r\n";
    }
    return "$" + to_string(value.length()) + "\r\n" + value + "\r\n";

    // chrono::steady_clock::time_point expiration_time =
    //     valueObj.expiration_time;

    // If expiration time is not set, return the value
    // if (expiration_time == chrono::steady_clock::time_point())
    // {
    //     return "$" + to_string(value.length()) + "\r\n" + value + "\r\n";
    // }

    // // Passive expiration
    // if (expiration_time < chrono::steady_clock::now())
    // {
    //     storage.erase(key);
    //     // or $-1\r\n
    //     return "$-1\r\n";
    // }

    // return "$" + to_string(value.length()) + "\r\n" + value + "\r\n";
}
