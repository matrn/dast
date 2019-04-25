#ifndef DAST_H_
#define DAST_H_

#include <sys/inotify.h>	/* inotify_* */
#include <sys/wait.h>   //fork wait
#include <linux/limits.h>	/* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>   /* flock */

#include <errno.h>
#include <limits.h>   /* for strtol limits */
#include <time.h>   /* unix time stamp */

#include <libgen.h>   /* basename, dirname */


typedef unsigned char byte;
typedef char s_byte;
typedef void (*callback_func)();
typedef struct { FILE * file; FILE * pidfile; } DSFILE;


extern char OLPD[3];
extern char OLUD[3];
extern char MLUD[3];

#define DT_UNKNOWN_VAR -1
//#define DT_
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))


//#define PIDFILE_END ".dast"
pid_t parent_pid;

int ifd, len, namlen;  // iwd
struct inotify_event *iev;
char buf[BUF_LEN] __attribute__ ((aligned(8)));
char *bufp;

int dast_watched_size;
char ** dast_watched_name;
callback_func * dast_watched_callback;
FILE ** dast_watched_pidfile;


s_byte dast_init();
s_byte dast_watch_dir(char * dir_name);
void dast_watch(char * filename, callback_func func);

void dast_run();
void dast_cleanup();

s_byte dast_open_rw(char * filename, DSFILE * file);   /* open file for reading and writting */
s_byte dast_open_ra(char * filename, DSFILE * file);   /* open file for reading and appending */

s_byte open_rw(char * filename, FILE ** file);

void dast_close(DSFILE dsfile);


s_byte dast_write_pid(pid_t pid, FILE * file);
pid_t dast_read_pid(FILE * file);


s_byte dast_read(char ** data, DSFILE file);
s_byte dast_write(char * data, DSFILE file);

ssize_t dast_read_var(char separators[2], char * var_name, char ** var_data, DSFILE file);
s_byte dast_write_var(char separators[2], char * var_name, char * var_data, DSFILE file);   /* projdou se všechny proměnné v souboru, zároveň se budou ukládat, pokud tahle proměnná existuje, tak se popupraví a vše se zapíše a flushne, pokud ne, přidá se nakonec, vše se zapíše a flushne */


/* -----helpers.c----- */
void dast_add_time(char delimiter, char * data, char ** output);
s_byte dast_parse_time(char delimiter, char * input, long * time, char ** data);

byte in_str(char * input, char character);
ssize_t get_pos(char * input, char character);

char * generate_pidfile_name(char * main_file_name);
/* -----helpers.c----- */

#endif