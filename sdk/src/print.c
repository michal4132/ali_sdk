#include <print.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <uart.h>

void pr(char *str) {
    int i = 0;
    while (str[i]) {
        uart_write_char(0, str[i++]);
    }
    uart_write_char(0, '\n');
}

// Reverses a string 'str' of length 'len'
void reverse(char *str, int len) {
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

int intToStr(int x, char str[], int d) {
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating-point/double number to a string.
void ftoa(float n, char *res, int afterpoint) {
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

int kprintf(char *str, ...) {
    va_list vl;
    int64_t i = 0, j = 0;
    char buff[100] = {0}, tmp[20];
    char *str_arg;

    va_start(vl, str);
    while (str && str[i]) {
        if (str[i] == '%') {
            i++;
            switch (str[i]) {
            /* Convert char */
            case 'c': {
                buff[j] = (char)va_arg(vl, int);
                j++;
                break;
            }
            /* Convert decimal */
            case 'd': {
                itoa(va_arg(vl, int), tmp, 10);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
            /* Convert unsigned decimal */
            case 'u': {
                itoa(va_arg(vl, uint32_t), tmp, 10);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
            case 'f': {
                ftoa(va_arg(vl, double), tmp, 6);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
            /* Convert hex */
            case 'x': {
                itoa(va_arg(vl, int), tmp, 16);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
                /* Convert unsigned hex */
            case 'a': {
                itoa(va_arg(vl, uint32_t), tmp, 16);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
            /* Convert octal */
            case 'o': {
                itoa(va_arg(vl, int), tmp, 8);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }
            /* copy string */
            case 's': {
                str_arg = va_arg(vl, char *);
                pr(str_arg);
                strcpy(&buff[j], str_arg);
                j += strlen(str_arg);
                break;
            }
            }
        } else {
            buff[j] = str[i];
            j++;
        }
        i++;
    }
    fwrite(buff, j, 1, stdout);
    va_end(vl);
    return j;
}
