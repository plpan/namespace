#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

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
    execv(child_args[0], child_args);
    printf("Ooops\n");
    return 1;
}

int main() {
	// [parent] init
	pipe(checkpoint);

    printf(" - [%5d] Hello ?\n", getpid());
    int child_pid = clone(child_main, child_stack+STACK_SIZE,
        CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD, NULL);

	// long init job
	sleep(2);
	// signal 'done'
	close(checkpoint[1]);

    waitpid(child_pid, NULL, 0);
    return 0;
}

