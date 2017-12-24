#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define STACK_SIZE (1024 * 1024)

// global status
int checkpoint[2];

static char child_stack[STACK_SIZE];

char* const child_args[] = {
    "/bin/bash",
    NULL
};

// 1. 在parent中关闭写
// 2. 在child中等待EOF

int child_main(void* arg) {
	// [child] wait
	char c; // stub char
	// [child] init sync primitive
	close(checkpoint[1]);
	// wait...
	read(checkpoint[0], &c, 1);

    printf(" - [%5d] World !\n", getpid());
	sethostname("stupig", 6);

	// remount "/proc" to get accurate "top" & "ps" output
	mount("proc", "/proc", "proc", 0, NULL);

	// setup network
	system("ip link set lo up");
	system("ip link set veth1 up");
	system("ip addr add 169.254.1.2/30 dev veth1");

    execv(child_args[0], child_args);
    printf("Ooops\n");
    return 1;
}

int main() {
	// [parent] init
	pipe(checkpoint);

    printf(" - [%5d] Hello ?\n", getpid());
    int child_pid = clone(child_main, child_stack+STACK_SIZE,
        CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD, NULL);

	// long init job
	sleep(2);

	// create a veth pair
	char *cmd;
	asprintf(&cmd, "ip link set veth1 netns %d", child_pid);
	system("ip link add veth0 type veth peer name veth1");
	system(cmd);
	system("ip link set veth0 up");
	system("ip addr add 169.254.1.1/30 dev veth0");
	free(cmd);

	// signal 'done'
	close(checkpoint[1]);

    waitpid(child_pid, NULL, 0);
    return 0;
}

