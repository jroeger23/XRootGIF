#ifndef __DAEMON_H_
#define __DAEMON_H_

typedef void daemon_task_t(void*);

/**
 * Execute each function in Null-terminated array tasks from 0..n with argv[i]
 */
int daemon_run(daemon_task_t **tasks, void **argv);

#endif // __DAEMON_H_
