#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>

int main(int argc, char *argv[])
{
    long ret;
    if (argc < 2) {
        fprintf(stderr, "specify pid\n");
        exit(1);
    }

    pid_t pid = atoi(argv[1]);
    printf("attach to %d\n", pid);

    ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if (ret < 0) {
        perror("failed to attach");
        exit(1);
    }
    printf("attach to %d (ret %ld)\n", pid, ret);
    sleep(5);

    ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    if (ret < 0) {
        perror("failed to detach");
        exit(1);
    }
    printf("detached from %d (ret: %ld)\n", pid, ret);

    return 0;
}
