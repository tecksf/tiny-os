#include <stdio.h>
#include "syscall.h"

int main(void)
{
    printf("Hello world!!.\n");
    printf("I am process %d.\n", getpid());
    printf("hello pass.\n");
    return 0;
}
