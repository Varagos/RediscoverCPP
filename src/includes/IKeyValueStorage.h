#pragma once

#include <string>
#include <optional>
#include <chrono>

using std::optional;
using std::string;

struct ValueWithExpiry
{
    std::string value;
    std::chrono::steady_clock::time_point expiration_time;
};

class IKeyValueStorage
{
public:
    virtual void set(string &key, string &value, const optional<std::chrono::milliseconds> &expiry) = 0;
    virtual optional<string> get(string &key) = 0;
    // virtual void remove(const string &key) = 0;
};