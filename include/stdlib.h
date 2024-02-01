#ifndef MPX_STDLIB_H
#define MPX_STDLIB_H

/**
 @file stdlib.h
 @brief A subset of standard C library functions.
*/

/**
 Convert an ASCII string to an integer
 @param s A NUL-terminated string
 @return The value of the string converted to an integer
*/
int atoi(const char *s);

/**
 Convert an integer to an ASCII string
 @param num The integer to convert
 @param str The resulting string buffer
 @param base The base (2 for binary, 10 for decimal, etc.)
 @return Pointer to the resulting string buffer
*/
char* itoa(int num, char* str, int base);

/**
 Format and print a string
 @param str The output string buffer
 @param format The format string
 @param ... The values to format and print
 @return Number of characters printed
*/
int sprintf(char *str, const char *format, ...);

/**
 Reverse a string in place
 @param str The string to reverse
 @param length The length of the string to reverse
*/
void reverse(char str[], int length);

#endif
