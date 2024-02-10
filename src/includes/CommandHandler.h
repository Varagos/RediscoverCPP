// CommandHandler.h
#pragma once
#include "IKeyValueStorage.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class CommandHandler
{
public:
    explicit CommandHandler(IKeyValueStorage *storage);
    string handleCommand(const string &cmd, vector<string> &args);

private:
    IKeyValueStorage *storage;

    string handlePing(const vector<string> &args);

    string handleEcho(const vector<string> &args);

    string handleSet(const vector<string> &args);

    string handleGet(const vector<string> &args);
};
