#ifndef DAST_H_
#define DAST_H_

#include <sys/inotify.h>	/* inotify_* */
#include <sys/wait.h>   //fork wait
#include <linux/limits.h>	/* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned char byte;
typedef char s_byte;
typedef void (*callback_func)();


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int ifd, len, namlen;  // iwd
struct inotify_event *iev;
char buf[BUF_LEN] __attribute__ ((aligned(8)));
char *bufp;

int dast_watched_size;
char ** dast_watched_name;
callback_func * dast_watched_callback;


s_byte dast_init();
void dast_watch(char *, callback_func);
s_byte dast_watch_dir(char *);
void dast_run();
void dast_cleanup();

#endif