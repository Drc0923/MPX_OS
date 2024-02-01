#include <ctype.h>

int isspace(int c)
{
	return (c == ' ' || c == '\n' || c == '\r' || c == '\f' || c == '\t' || c == '\v');
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

