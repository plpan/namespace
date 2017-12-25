#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while(0)

static char child_stack[STACK_SIZE];

int child_main(void* arg) {
	cap_t caps;
	for(;;) {
		printf("eUID = %ld, eGID = %ld; ",
				(long) geteuid(), (long) getegid());
		caps = cap_get_proc();
		// 展示capabilities
		printf("capabilities: %s\n", cap_to_text(caps, NULL));

		if (arg == NULL) {
			break;
		}

		sleep(5);
	}
    return 0;
}

int main(int argc, char *argv[]) {
	// create child, child commences execution in child_main()
    int pid = clone(child_main, child_stack+STACK_SIZE,
        CLONE_NEWUSER | SIGCHLD, argv[1]);
	// if kernel do not support USER_NS, exit
	if (pid == -1) errExit("clone");
	// parnet falls through to here, wait for child
    if (waitpid(pid, NULL, 0) == -1) errExit("waitpid");
    exit(EXIT_SUCCESS);
}

