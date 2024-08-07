# Main.c
This program reads from standard input whole non-negative decimal numbers separated by spaces or line breaks (or other non-numeric characters) and writes them to standard output 
in hexadecimal form separated by line ends. The program is not using any standard library such as libc. We assume that the maximum value of the number will be 2^32-1.

In other words, it's program that works like the program below, but compiles with compiler switches
```
-ffreestanding -fno-stack-protector -nostdlib -nostdinc -static -m32 -Wall -g -O2
```
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
# Nbrk.c

In the OS Nova core (http://hypervisor.org/), I implemented the nbrk system call with a prototype:

void *nbrk(void *address)

This system call sets the end of the data segment in the address space of the process (so-called program break or just break) to the address given by the address parameter. 
This increases or decreases the amount of allocated memory that the program can use to run. Break is the first address after the end of the mapped data segment.

The solution meets the following requirements:
* After a successful return from the system call, break is set to address. This means that a user program can use memory from address 0x1000 to an address one less than address.
* The program will not allow access to pages starting at a higher or equal address.
* Break must not be set to a lower value than its value when the program is started. This would deprive the program of part of its code or data.
* Break must not be set to a higher value than 0xBFFFF000. This would allow the application to overwrite its stack or even the kernel, which is mapped from address 0xC0000000 upwards.
* On successful completion, the original break value before the system call was executed is returned. 0 is returned on error.
* If address is NULL (0), it is not an error and the break value is not changed. This call is only used to find the current value of break.
* Any call to nbrk must not cause the system to "crash".
* The system call ABI will be as follows:
        - input: AL=3, ESI=address,
        - output: EAX=return value.
* Newly allocated memory will be initialized to zero.
* When decrementing the break value, inaccessible memory will be deallocated (and unmapped) so that it can be re-allocated later.
* In the event of a memory allocation error during a system call, the address space of the user application will not be changed and the partially allocated memory will be deallocated.
* The compiler does not issue any warnings during compilation.
