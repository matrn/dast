#include <stdio.h>
#include <unistd.h>		/* read() */

#include "dast.h"
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
	if(dast_init() != 0){
		perror("dast init");
	}	
	

	
	if(dast_watch_dir(".") < 0){
		perror("inotify_add_watch");
		return 5;
	}

	/*if(dast_watch_dir("/home/matej") < 0){
		perror("inotify_add_watch");
		return 5;
	}
	*/
	dast_watch("t1", callback_1);
	dast_watch("t2", callback_2);

	dast_run();

	FILE * file1;
	char * content;

	dast_open_rw("t1", &file1);


	char dd[3] = {'A', 'D', 'E'};

	//printf("Retudned: %d\n", dast_write_var(dd, "aa", "test", &file1));
	
	//content = malloc(50);

	ssize_t len = 0;
	if((len = dast_read_var(dd, "test_var", &content, &file1)) != -1){
		printf("Content >%s<\n", content);
		printf("LEN: %ld, reutrned len: %ld\n", strlen(content), len);
		free(content);
	}
	else{
		puts("Unknown variable");
	}
	//printf("Returned: %d\n", dast_write_var(MLUD, "test_var", "hey\nwhatsuuup? \n lol, coolll", &file1));

	//free(content);
	while(1){
		//puts("tick");
		sleep(1);
	}

	dast_close(&file1);

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