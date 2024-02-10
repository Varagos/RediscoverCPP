#pragma once
#include "IKeyValueStorage.h"
#include <map>
#include <string>
#include <optional>
#include <chrono>

using namespace std;

class InMemoryStorage : public IKeyValueStorage
{
private:
    map<string, ValueWithExpiry> storage;

public:
    void set(string &key, string &value, const optional<chrono::milliseconds> &expiry);
    optional<string> get(string &key);

    // void remove(const string &key);
};