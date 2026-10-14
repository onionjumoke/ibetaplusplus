#include "J_SajParser.h"
#include "J_JsonListener.h"
#include "J_PositionTrackingPushbackReader.h"

#include <cstring>
#include <sstream>
#include <stdexcept>

void J_SajParser::parse(std::istream& in, J_JsonListener* listener)
{
    J_PositionTrackingPushbackReader reader(&in);
    char c = (char)skipWhitespace(reader);
    switch (c)
    {
    case '{':
        reader.unreadChar(c);
        listener->startDocument();
        parseObject(reader, listener);
        break;
    case '[':
        reader.unreadChar(c);
        listener->startDocument();
        parseArray(reader, listener);
        break;
    default:
        throw J_InvalidSyntaxException(
            std::string("Expected either [ or { but got [") + c + "].", &reader);
    }
    int trailing = skipWhitespace(reader);
    if (trailing != -1)
        throw J_InvalidSyntaxException(
            std::string("Got unexpected trailing character [") + (char)trailing + "].", &reader);
    listener->endDocument();
}

void J_SajParser::parseArray(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener)
{
    char c = (char)skipWhitespace(reader);
    if (c != '[')
        throw J_InvalidSyntaxException(
            std::string("Expected object to start with [ but got [") + c + "].", &reader);

    listener->startArray();

    char peek = (char)skipWhitespace(reader);
    reader.unreadChar(peek);

    if (peek != ']')
        parseValue(reader, listener);

    bool done = false;
    while (!done)
    {
        char sep = (char)skipWhitespace(reader);
        switch (sep)
        {
        case ',':
            parseValue(reader, listener);
            break;
        case ']':
            done = true;
            break;
        default:
            throw J_InvalidSyntaxException(
                std::string("Expected either , or ] but got [") + sep + "].", &reader);
        }
    }
    listener->endArray();
}

void J_SajParser::parseObject(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener)
{
    char c = (char)skipWhitespace(reader);
    if (c != '{')
        throw J_InvalidSyntaxException(
            std::string("Expected object to start with { but got [") + c + "].", &reader);

    listener->startObject();

    char peek = (char)skipWhitespace(reader);
    reader.unreadChar(peek);

    if (peek != '}')
        parseField(reader, listener);

    bool done = false;
    while (!done)
    {
        char sep = (char)skipWhitespace(reader);
        switch (sep)
        {
        case ',':
            parseField(reader, listener);
            break;
        case '}':
            done = true;
            break;
        default:
            throw J_InvalidSyntaxException(
                std::string("Expected either , or } but got [") + sep + "].", &reader);
        }
    }
    listener->endObject();
}

void J_SajParser::parseField(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener)
{
    char c = (char)skipWhitespace(reader);
    if (c != '"')
        throw J_InvalidSyntaxException(
            std::string("Expected object identifier to begin with [\"] but got [") + c + "].", &reader);
    reader.unreadChar(c);

    std::string key = parseString(reader);
    listener->startField(key);

    char colon = (char)skipWhitespace(reader);
    if (colon != ':')
        throw J_InvalidSyntaxException(
            std::string("Expected object identifier to be followed by : but got [") + colon + "].", &reader);

    parseValue(reader, listener);
    listener->endField();
}

void J_SajParser::parseValue(J_PositionTrackingPushbackReader& reader, J_JsonListener* listener)
{
    char c = (char)skipWhitespace(reader);
    switch (c)
    {
    case '"':
        reader.unreadChar(c);
        listener->stringValue(parseString(reader));
        break;

    case 't':
    {
        char buf[3];
        int n = reader.readChars(buf, 3);
        if (n != 3 || buf[0] != 'r' || buf[1] != 'u' || buf[2] != 'e')
        {
            reader.untrackChars(buf, n);
            throw J_InvalidSyntaxException(
                std::string("Expected 't' to be followed by [[r, u, e]], but got something else."), &reader);
        }
        listener->trueValue();
        break;
    }

    case 'f':
    {
        char buf[4];
        int n = reader.readChars(buf, 4);
        if (n != 4 || buf[0] != 'a' || buf[1] != 'l' || buf[2] != 's' || buf[3] != 'e')
        {
            reader.untrackChars(buf, n);
            throw J_InvalidSyntaxException(
                std::string("Expected 'f' to be followed by [[a, l, s, e]], but got something else."), &reader);
        }
        listener->falseValue();
        break;
    }

    case 'n':
    {
        char buf[3];
        int n = reader.readChars(buf, 3);
        if (n != 3 || buf[0] != 'u' || buf[1] != 'l' || buf[2] != 'l')
        {
            reader.untrackChars(buf, n);
            throw J_InvalidSyntaxException(
                std::string("Expected 'n' to be followed by [[u, l, l]], but got something else."), &reader);
        }
        listener->nullValue();
        break;
    }

    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        reader.unreadChar(c);
        listener->numberValue(parseNumber(reader));
        break;

    case '{':
        reader.unreadChar(c);
        parseObject(reader, listener);
        break;

    case '[':
        reader.unreadChar(c);
        parseArray(reader, listener);
        break;

    default:
        throw J_InvalidSyntaxException(
            std::string("Invalid character at start of value [") + c + "].", &reader);
    }
}

std::string J_SajParser::parseNumber(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    char c = (char)reader.readChar();
    if (c == '-')
        result += '-';
    else
        reader.unreadChar(c);
    result += parsePositiveNumber(reader);
    return result;
}

std::string J_SajParser::parsePositiveNumber(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    char c = (char)reader.readChar();
    if (c == '0')
    {
        result += '0';
        result += parseFractional(reader);
        result += parseExponent(reader);
    }
    else
    {
        reader.unreadChar(c);
        result += parseNonZeroDigit(reader);
        result += parseMoreDigits(reader);
        result += parseFractional(reader);
        result += parseExponent(reader);
    }
    return result;
}

char J_SajParser::parseNonZeroDigit(J_PositionTrackingPushbackReader& reader)
{
    char c = (char)reader.readChar();
    if (c >= '1' && c <= '9')
        return c;
    throw J_InvalidSyntaxException(
        std::string("Expected a digit 1 - 9 but got [") + c + "].", &reader);
}

char J_SajParser::parseDigit(J_PositionTrackingPushbackReader& reader)
{
    char c = (char)reader.readChar();
    if (c >= '0' && c <= '9')
        return c;
    throw J_InvalidSyntaxException(
        std::string("Expected a digit 1 - 9 but got [") + c + "].", &reader);
}

std::string J_SajParser::parseMoreDigits(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    bool done = false;
    while (!done)
    {
        char c = (char)reader.readChar();
        if (c >= '0' && c <= '9')
            result += c;
        else
        {
            reader.unreadChar(c);
            done = true;
        }
    }
    return result;
}

std::string J_SajParser::parseFractional(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    char c = (char)reader.readChar();
    if (c == '.')
    {
        result += '.';
        result += parseDigit(reader);
        result += parseMoreDigits(reader);
    }
    else
    {
        reader.unreadChar(c);
    }
    return result;
}

std::string J_SajParser::parseExponent(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    char c = (char)reader.readChar();
    if (c == '.' || c == 'E' || c == 'e')
    {
        result += 'E';
        result += parseSign(reader);
        result += parseDigit(reader);
        result += parseMoreDigits(reader);
    }
    else
    {
        reader.unreadChar(c);
    }
    return result;
}

std::string J_SajParser::parseSign(J_PositionTrackingPushbackReader& reader)
{
    std::string result;
    char c = (char)reader.readChar();
    if (c == '+' || c == '-')
        result += c;
    else
        reader.unreadChar(c);
    return result;
}

std::string J_SajParser::parseString(J_PositionTrackingPushbackReader& reader)
{
    char open = (char)reader.readChar();
    if (open != '"')
        throw J_InvalidSyntaxException(
            std::string("Expected [\"] but got [") + open + "].", &reader);

    std::string result;
    bool done = false;
    while (!done)
    {
        char c = (char)reader.readChar();
        switch (c)
        {
        case '"':
            done = true;
            break;
        case '\\':
            result += parseEscapeChar(reader);
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}

char J_SajParser::parseEscapeChar(J_PositionTrackingPushbackReader& reader)
{
    char c = (char)reader.readChar();
    switch (c)
    {
    case '"':  return '"';
    case '\\': return '\\';
    case '/':  return '/';
    case 'b':  return '\b';
    case 'f':  return '\f';
    case 'n':  return '\n';
    case 'r':  return '\r';
    case 't':  return '\t';
    case 'u':  return (char)parseUnicode(reader);
    default:
        throw J_InvalidSyntaxException(
            std::string("Unrecognised escape character [") + c + "].", &reader);
    }
}

int J_SajParser::parseUnicode(J_PositionTrackingPushbackReader& reader)
{
    char buf[4];
    int n = reader.readChars(buf, 4);
    if (n != 4)
        throw J_InvalidSyntaxException(
            std::string("Expected a 4 digit hexidecimal number but got only ") + std::to_string(n) + ".", &reader);

    int value = 0;
    for (int i = 0; i < 4; i++)
    {
        char h = buf[i];
        int digit;
        if (h >= '0' && h <= '9')      digit = h - '0';
        else if (h >= 'a' && h <= 'f') digit = h - 'a' + 10;
        else if (h >= 'A' && h <= 'F') digit = h - 'A' + 10;
        else
        {
            reader.untrackChars(buf, 4);
            throw J_InvalidSyntaxException(
                std::string("Unable to parse as a hexidecimal number."), &reader);
        }
        value = value * 16 + digit;
    }
    return value;
}

int J_SajParser::skipWhitespace(J_PositionTrackingPushbackReader& reader)
{
    int ch;
    do
    {
        ch = reader.readChar();
    }
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
    return ch;
}
