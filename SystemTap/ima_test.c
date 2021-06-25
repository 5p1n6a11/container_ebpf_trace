#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
    open("./context.stp", O_RDONLY, 0);

    return 0;
}
