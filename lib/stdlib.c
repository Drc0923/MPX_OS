#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include "stdlib.h"



int atoi(const char *s) {
    int res = 0;
    char sign = '+';

    while (isspace(*s)) {
        s++;
    }

    if (*s == '-' || *s == '+') {
        sign = *s;
        s++;
    }

    while (isdigit(*s)) {
        res = res * 10 + (*s - '0');
        s++;
    }

    return (sign == '-') ? -res : res;
}

char* itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num /= base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse(str, i);

    return str;
}

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    const char *s;
    char temp[50];
    char c;
    int d;
    char *start = str;

    while (*format) {
        if (*format != '%') {
            *str++ = *format++;
            continue;
        }

        format++; // move past '%'

        switch (*format) {
            case 's':
                s = va_arg(args, const char *);
                while (*s) {
                    *str++ = *s++;
                }
                format++;
                break;
            case 'd':
                d = va_arg(args, int);
                itoa(d, temp, 10);
                s = temp;
                while (*s) {
                    *str++ = *s++;
                }
                format++;
                break;
            case 'c':
                c = (char)va_arg(args, int);
                *str++ = c;
                format++;
                break;
            default:
                *str++ = *format++;
                break;
        }
    }

    *str = '\0';
    va_end(args);
    return (str - start);
}
