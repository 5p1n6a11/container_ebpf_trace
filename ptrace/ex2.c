#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>

void peek_and_output(pid_t pid, long long addr, long long size, int fd);

int main(int argc, char *argv[])
{
    int status;

    if (argc < 2) {
        fprintf(stderr, "specify pid\n");
        exit(1);
    }

    pid_t pid = atoi(argv[1]);
    printf("attach to %d\n", pid);

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        perror("failed to attach");
        exit(1);
    }

    struct user_regs_struct regs;

    ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);

    int is_enter_stop = 0;
    long prev_orig_rax = -1;

    while (1) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            break;
        } else if (WIFSIGNALED(status)) {
            printf("terminated by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0x80)) {
            // printf("stopped by signal %d\n", WSTOPSIG(status));
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            is_enter_stop = prev_orig_rax == regs.orig_rax ? !is_enter_stop : 1;
            prev_orig_rax = regs.orig_rax;
            if (is_enter_stop && regs.orig_rax == SYS_write) {
                // printf("%lld %lld %lld %lld %lld\n", regs.orig_rax, regs.rax, regs.rsi, regs.rdx, regs.rdi);
                peek_and_output(pid, regs.rsi, regs.rdx, (int)regs.rdi);
            }
        }

        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }

    return 0;
}

void peek_and_output(pid_t pid, long long addr, long long size, int fd)
{
    if (fd != 1 && fd != 2) {
        return;
    }

    char *bytes = malloc(size + sizeof(long));
    int i;
    for (i = 0; i < size; i += sizeof(long)) {
        long data = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (data == -1) {
            printf("failed to peek data\n");
            free(bytes);
            return;
        }
        memcpy(bytes + i, &data, sizeof(long));
    }
    bytes[size] = '\0';
    write(fd == 2 ? 2 : 1, bytes, size);
    fflush(fd == 2 ? stderr : stdout);
    free(bytes);
}

