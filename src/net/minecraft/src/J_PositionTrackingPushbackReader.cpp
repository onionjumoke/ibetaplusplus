#include "J_PositionTrackingPushbackReader.h"

J_PositionTrackingPushbackReader::J_PositionTrackingPushbackReader(std::istream* in)
    : in(in)
    , col(0)
    , line(1)
    , lastWasCR(false)
{
}

void J_PositionTrackingPushbackReader::unreadChar(char c)
{
    col--;
    if (col < 0)
        col = 0;
    pushback.push_front(c);
}

void J_PositionTrackingPushbackReader::untrackChars(const char* buf, int len)
{
    col -= len;
    if (col < 0)
        col = 0;
}

int J_PositionTrackingPushbackReader::readChar()
{
    int ch;
    if (!pushback.empty())
    {
        ch = (unsigned char)pushback.front();
        pushback.pop_front();
    }
    else
    {
        ch = in->get();
        if (!in->good())
            return -1;
    }
    trackChar(ch);
    return ch;
}

int J_PositionTrackingPushbackReader::readChars(char* buf, int len)
{
    int count = 0;
    for (int i = 0; i < len; i++)
    {
        int ch = readChar();
        if (ch == -1)
            break;
        buf[i] = (char)ch;
        count++;
    }
    return count;
}

void J_PositionTrackingPushbackReader::trackChar(int ch)
{
    if (ch == '\r')
    {
        col = 0;
        line++;
        lastWasCR = true;
    }
    else if (ch == '\n' && !lastWasCR)
    {
        col = 0;
        line++;
        lastWasCR = false;
    }
    else
    {
        col++;
        lastWasCR = false;
    }
}

int J_PositionTrackingPushbackReader::getColumn()
{
    return col;
}

int J_PositionTrackingPushbackReader::getLine()
{
    return line;
}
