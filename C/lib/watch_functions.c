#include "dast.h"

s_byte dast_init(){
	if((ifd = inotify_init()) < 0) {
		return -1;
	}

	return 0;
}


void dast_watch(char * filename, callback_func func){
	dast_watched_size ++;

	dast_watched_name = realloc(dast_watched_name, dast_watched_size * sizeof(char *));
	dast_watched_callback =  realloc(dast_watched_callback, dast_watched_size * sizeof(callback_func));
	
	dast_watched_name[dast_watched_size - 1] = malloc(strlen(filename) + 1);
	strcpy(dast_watched_name[dast_watched_size - 1], filename);

	dast_watched_callback[dast_watched_size - 1] = func;

	//(*func)();
}

void dast_run(){
	pid_t cpid;

	/* Forking a child */
	cpid = fork();

	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}


	if (cpid == 0) {	/* child */
		puts("child");
		for(;;){
			if ((len = read(ifd, &buf, BUF_LEN)) < 0) {
				perror("read iev");
				//return 5;
			}
					
		
			for(bufp = buf; bufp < buf + len; ){
				iev = (struct inotify_event *)bufp;

				printf("inotify for %s: ", "aa");
				printf("Mask: ");
				if (iev->mask & IN_ACCESS)		 printf("ACCESS ");
				if (iev->mask & IN_ATTRIB)		 printf("ATTRIB ");
				if (iev->mask & IN_CLOSE_NOWRITE)printf("CLOSE_NOWRITE ");
				if (iev->mask & IN_CLOSE_WRITE)	 printf("CLOSE_WRITE ");
				if (iev->mask & IN_CREATE)		 printf("CREATE ");
				if (iev->mask & IN_DELETE)		 printf("DELETE ");
				if (iev->mask & IN_DELETE_SELF)	 printf("DELETE_SELF ");
				if (iev->mask & IN_IGNORED)		 printf("IGNORED ");
				if (iev->mask & IN_ISDIR)		 printf("ISDIR ");
				if (iev->mask & IN_MODIFY)	     printf("MODIFY ");
				if (iev->mask & IN_MOVE_SELF)	 printf("MOVE_SELF ");
				if (iev->mask & IN_MOVED_FROM)	 printf("MOVED_FROM ");
				if (iev->mask & IN_MOVED_TO)	 printf("MOVED_TO ");
				if (iev->mask & IN_OPEN)		 printf("OPEN ");
				if (iev->mask & IN_Q_OVERFLOW)	 printf("Q_OVERFLOW ");
				if (iev->mask & IN_UNMOUNT)		 printf("UNMOUNT ");

				if (iev->mask & IN_CLOSE_WRITE || iev->mask & IN_MODIFY){
					for(int a = 0; a < dast_watched_size; a ++){
						if(strcmp(dast_watched_name[a], iev->name) == 0) (*dast_watched_callback[a])();
					}
					printf("CLOSE_WRITE ");

				}

				printf("Cookie: %d Name Len: %d", iev->cookie, iev->len);
				//if (namlen == iev->len) {
					printf(" Name: %s", iev->name);
				//}
				puts("");
				bufp += sizeof(struct inotify_event) + iev->len;
			}
		}
		_exit(EXIT_SUCCESS);

	} else {			/* parent */
		puts("parent");
		//exit(EXIT_SUCCESS);
	}

}

s_byte dast_watch_dir(char * dir_name){
	if(inotify_add_watch(ifd, dir_name, IN_CLOSE_WRITE | IN_CLOSE_NOWRITE | IN_MODIFY) < 0){  //IN_ALL_EVENTS
		return -1;
	}

	return 0;
}


void dast_cleanup(){
	wait(NULL);			 /* Wait for child */
	free(dast_watched_name);
	free(dast_watched_callback);
}