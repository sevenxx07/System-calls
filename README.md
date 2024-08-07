# Main.c
This program reads from standard input whole non-negative decimal numbers separated by spaces or line breaks (or other non-numeric characters) and writes them to standard output 
in hexadecimal form separated by line ends. The program is not using any standard library such as libc. We assume that the maximum value of the number will be 2^32-1.

In other words, it's program that works like the program below, but compiles with compiler switches

-ffreestanding -fno-stack-protector -nostdlib -nostdinc -static -m32 -Wall -g -O2
```
#include <stdio.h>

int main()
{
    unsigned num;
    while (scanf("%u", &num) == 1)
        printf("0x%x\n", num);
    return 0;
}
```
