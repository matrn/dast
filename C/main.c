#include <stdio.h>
#include <unistd.h>		/* read() */

#include "dast.h"
#include <sys/inotify.h>	/* inotify_* */

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
	

	
	if(dast_watch_dir(".") != 0){
		perror("inotify_add_watch");
		return 5;
	}

	dast_watch("t1", callback_1);
	dast_watch("t2", callback_2);

	dast_run();
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