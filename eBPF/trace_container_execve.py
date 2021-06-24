#!/usr/bin/env python3

from bcc import BPF

bpf_text = """
#include <linux/sched.h>
#include <linux/nsproxy.h>
#include <linux/pid_namespace.h>
#include <linux/proc_ns.h>
#include <linux/utsname.h>

int trace_ret_execve(struct pt_regs *ctx)
{
    u32 pid;
    struct task_struct *task;
    char comm[TASK_COMM_LEN];

    pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&comm, sizeof(comm));

    task = (struct task_struct *) bpf_get_current_task();
    struct pid_namespace *pns = (struct pid_namespace *) task->nsproxy->pid_ns_for_children;

    if (pns->ns.inum == PROC_PID_INIT_INO) {
        return 0;
    }

    struct uts_namespace *uns = (struct uts_namespace *) task->nsproxy->uts_ns;

    bpf_trace_printk("name: %s\\n", uns->name.nodename);
    bpf_trace_printk("comm: %s, ns: %lld\\n", comm, pns->ns.inum);
    return 0;
}
"""

bpf = BPF(text=bpf_text)
execve_fnname = bpf.get_syscall_fnname("execve")
bpf.attach_kretprobe(event=execve_fnname, fn_name="trace_ret_execve")

while 1:
    try:
        bpf.trace_print()
    except KeyboardInterrupt:
        exit()

