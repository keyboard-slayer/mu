#include "ctype.h"

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    return c;
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}