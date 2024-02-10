#include "InMemoryStorage.h"

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

map<string, ValueWithExpiry> storage;

void InMemoryStorage::set(string &key, string &value, const optional<chrono::milliseconds> &expiry)
{
    ValueWithExpiry valueObj;
    valueObj.value = value;
    if (expiry)
    {
        valueObj.expiration_time = chrono::steady_clock::now() + expiry.value();
    }
    storage[key] = valueObj;
};

optional<string> InMemoryStorage::get(string &key)
{
    ValueWithExpiry valueObj = storage[key];
    string value = valueObj.value;
    chrono::steady_clock::time_point expiration_time = valueObj.expiration_time;

    // If expiration time is not set, return the value
    if (expiration_time == chrono::steady_clock::time_point())
    {
        return value;
    }

    // Passive expiration
    if (expiration_time < chrono::steady_clock::now())
    {
        storage.erase(key);
        return nullopt;
    }

    return value;
};
