#include <stdio.h>
#include <unistd.h>

void main()
{
    write(STDOUT_FILENO, "Hello, world!", 13);
}
