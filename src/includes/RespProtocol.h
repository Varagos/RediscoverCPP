// RespProtocol.h
#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

class RespProtocol
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
    // Encode a message
    static std::string encode(const std::string message);

    // Decode a message
    static std::string decode(const std::string message);

    // Tokenize a RESP message
    static std::vector<std::string> tokenizeRESP(const std::string &msg);

    static void printMessageWithVisibleNewlines(const char *msg);
};