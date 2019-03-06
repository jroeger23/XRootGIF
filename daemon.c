#include "daemon.h"

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

#include "globals.h"
#include "output.h"

void daemon_interrupt_handler(int i)
{
        do_anim = false;
}

int daemon_run(daemon_task_t **tasks, void **argv)
{
        pid_t pid;
        int index = 0;

        /* do daemon stuff */
        if( (pid = fork()) < 0)
                return EXIT_FAILURE;

        if(pid > 0)
                exit(EXIT_SUCCESS);

        if(setsid() < 0)
                return EXIT_FAILURE;

        signal(SIGINT, daemon_interrupt_handler);
        signal(SIGKILL, daemon_interrupt_handler);
        signal(SIGTERM, daemon_interrupt_handler);
        signal(SIGHUP, SIG_IGN); // Implement a reaload function maybe?

        if( (pid = fork()) < 0)
                return EXIT_FAILURE;
        if(pid > 0)
                exit(EXIT_SUCCESS);

        /* close file handles */
        for(int i = _SC_OPEN_MAX; i >= 0; --i) {
                close(i);
        }

        /* execute daemon tasks */
        while(tasks[index] != NULL) {
                tasks[index](argv[index]);
                index += 1;
        }

        return EXIT_SUCCESS;
}
