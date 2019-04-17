#include <stdio.h>
#include <unistd.h>		/* read() */

#include <dast.h>
#include <sys/file.h>

/*
		struct inotify_event {
			int			wd;			 Watch descriptor
			uint32_t mask;		 Mask of events
			uint32_t cookie;	 Unique cookie associating related
												 events (for rename(2))
			uint32_t len;			Size of 'name' field
			char		 name[];	 Optional null-terminated name
		};
*/


void callback_1();
void callback_2();


int main(int argc, char ** argv){
	FILE * file1, * file2;
	char * content;
	ssize_t len = 0;
	s_byte rtn;

	/* initialize dast */
	if(dast_init() != 0){
		perror("dast init");
	}	
	
	/* add new directory to watch */
	if(dast_watch_dir(".") != 0){
		perror("inotify_add_watch");
		return 5;
	}

	/*if(dast_watch_dir("/home/matej") < 0){
		perror("inotify_add_watch");
		return 5;
	}
	*/

	/* add callbacks for specific files */
	dast_watch("test1", callback_1);
	dast_watch("test2", callback_2);

	/* run inotify daemon */
	dast_run();

	/* open files for read and write */
	dast_open_rw("test1", &file1);
	dast_open_rw("test2", &file2);

	char dd[3] = {'$', ',', ';'};
	
	/* try to read test_var variable */
	if((len = dast_read_var(OLPD, "test_var", &content, &file1)) != -1){
		printf(" 'test_var' content >%s<\n", content);
		//printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(content);
	}
	else{
		puts("Unknown variable 'test_var'");
	}

	puts("-------------------write---------------------");
	if((rtn = dast_write_var(MLUD, "test_var", "Hello World!", &file1)) != -1){
		if(rtn == 0) puts("rewritten only one line");
		if(rtn == 1) puts("rewritten file from position of variable to the end of file");
		if(rtn == 2) puts("added to the end of file");

		//printf("Returned: %d\n", rtn);
	}else{
		perror("data_write_var");
	}
	puts("-------------------write---------------------");

	/* try to read test_var variable */
	if((len = dast_read_var(OLPD, "test_var", &content, &file1)) != -1){
		printf(" 'test_var' content >%s<\n", content);
		//printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(content);
	}
	else{
		puts("Unknown variable");
	}

	char * tt;
	dast_add_time('-', "cool", &tt);
	printf("OUT >%s<\n", tt);
	printf("IN str: %d\n", in_str(tt, 'Z'));


	long tm = 0;
	char * str;
	//const char format = {'%', 'l', 'u', '-', '%', 'm', 's'};
	rtn = dast_parse_time('-', "556-lol", &tm, &str);
	printf("Returned: %d, Time >%lu< str >%s<\n", rtn, tm, str);
	free(str);
	free(tt);
	//printf("Returned: %d\n", dast_write_var(MLUD, "test_var", "hey\nwhatsuuup? \n lol, coolll", &file1));

	//free(content);
	/*
	while(1){
		//puts("tick");
		sleep(1);
	}
	*/
	sleep(1);
	puts("killing");
	kill(0, SIGKILL);   /* kill child */

	dast_close(&file1);
	dast_close(&file2);

	dast_cleanup();
	return 0;
}



void callback_1(){
	puts("");
	puts("Callback_1");
}


void callback_2(){
	puts("");
	puts("Callback_2");
}
