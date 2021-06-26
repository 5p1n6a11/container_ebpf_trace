#include <stdio.h>
#include <sys/syscall.h>

int main(int argc, char const *argv[])
{
    printf("%d\n", SYS_write);
    return 0;
}
