#ifndef DAST_H_
#define DAST_H_


#include <sys/inotify.h>	/* inotify_* */
#include <sys/wait.h>   /* fork wait */
#include <linux/limits.h>	/* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>   /* flock */
#include <errno.h>   /* perror */
#include <limits.h>   /* for strtol limits */
#include <time.h>   /* unix time stamp */
#include <libgen.h>   /* basename, dirname */

//#define DEBUG



typedef unsigned char byte;   /* number 0 - 255 */
typedef signed char s_byte;   /* number -127 - 127 */
typedef void (*callback_func)(char * dir_name, char * name, pid_t pid);   /* callback function */
typedef struct { FILE * file; FILE * pidfile; } DSFILE;   /* DSFILE - two files: file for variables and pid file */
typedef struct node {
	char * key;
	char * value;
	struct node * next;
} dict;


extern char OLPD[3];   /* one-line printable delimiter */
extern char OLUD[3];   /* one-line unprintable delimiter */
extern char MLUD[3];   /* multi-line unprintable delimiter */
extern char TPD;   /* time printable delimiter */ 
extern char TUD;   /* time unprintable delimiter */ 

#define ever ;;

#define UNKNOWN_WD -2   /* unknown watch descriptor */
#define UNKNOWN_VAR -2   /* variables not found - used in dast_read_var */
#define ERROR -1   /* some kind of error */
#define OK 0   /* OK :) */
#define W_ONE_LINE 0   /* rewritten only one line */
#define W_MORE_LINES 1   /* rewritten all lines from variable position */
#define A_END 2   /* added to the end of file */

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))


pid_t parent_pid;   /* PID of parent process, for saving some CPU instructions */


/* -----inotify variables----- */
int ifd, len, namlen;  // iwd
struct inotify_event * iev;
char buf[BUF_LEN] __attribute__ ((aligned(8)));
char * bufp;
/* -----inotify variables----- */

/* -----arrays for saving name of watched file, callback for specifci file, pidfile for watched file----- */
int dast_watched_size;   /* length of array */
char ** dast_watched_name;   /* name of watched file */
callback_func * dast_watched_callback;   /* callback */
FILE ** dast_watched_pidfile;   /* pidfile for specifci file */

int dast_pidfile_size;   /* length of array */
char ** dast_pidfile_name;   /* name of pidfile */
FILE ** dast_pidfile_fp;   /* file pointer of pidfile */

byte dast_dir_size;
char ** dast_dir_name;
int * dast_dir_wd;
/* -----arrays for saving name of watched file, callback for specifci file, pidfile for watched file----- */



/* -----watch_functions.c----- */
s_byte dast_init();
s_byte dast_watch_dir(char * dir_name);
s_byte dast_watch(char * filename, callback_func func);

s_byte dast_run();
void dast_cleanup();

s_byte dast_get_dir(int wd, char ** dir_name);
/* -----watch_functions.c----- */


/* -----file_functions.c----- */
s_byte open_rw(char * filename, FILE ** file);   /* open file for reading and writing */

s_byte dast_open_rw(char * filename, DSFILE * file);   /* open file for reading and writing & pidfile for reading and writing */
s_byte dast_open_rw_s(char * filename, char * pidfile_name, DSFILE * file);   /* specific - open file for reading and writing & pidfile for reading and writing with chooseable pidfile name */
//s_byte dast_open_ra(char * filename, DSFILE * file);   /* open file for reading and appending */

void dast_close(DSFILE dsfile);   /* close file & pidfile */


s_byte dast_write_pid(pid_t pid, FILE * file);   /* for writing PID to file */
pid_t dast_read_pid(FILE * file);   /* for reading PID from file */


s_byte dast_read(char ** data, DSFILE file);
s_byte dast_write(char * data, DSFILE file);

long dast_read_var(char separators[3], char * var_name, char ** var_data, DSFILE file);   /* reads variable */

s_byte dast_just_write_var(char separators[3], char * var_name, char * var_data, FILE * file);   /* just writes variable without flock, fflush, pidwrite and etc. */
s_byte dast_write_var(char separators[3], char * var_name, char * var_data, DSFILE file);   /* writes variable */
s_byte dast_write_vars(char separators[3], dict * vars, DSFILE file);   /* writes variables */
/* -----file_functions.c----- */


/* -----helpers.c----- */
s_byte dast_add_time(char delimiter, char * data, char ** output);
s_byte dast_parse_time(char delimiter, char * input, long * time, char ** data);

byte in_str(char * input, char character);   /* returns true or false */
ssize_t get_pos(char * input, char character);

char * generate_pidfile_name(char * main_file_name);

byte startsWith(char * string, char * prefix);   /* returns true or false */
byte dast_is_spc_name(char * name);   /* returns true or false */
byte dast_name_cmp(char * filename_in, char * filename_cmp);   /* returns true or false */

s_byte dast_get_array_pidfile(char * dir_name, char * name, FILE ** pidfile);
/* -----helpers.c----- */


/* -----dict.c----- */
s_byte dict_init(dict ** head);
s_byte dict_set(dict * head, char * key, char * value);
s_byte dict_get(dict * head, char * key, char ** value);
void dict_print_all(dict * head);

void dict_free(dict * head);
/* -----dict.c----- */


#endif