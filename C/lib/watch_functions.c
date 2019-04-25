#include "dast.h"



s_byte dast_init(){
	if((ifd = inotify_init()) < 0) {
		return -1;
	}

	return 0;
}


void dast_watch(char * filename, callback_func func){
	char * pidfile_name;

	dast_watched_size ++;

	dast_watched_name = realloc(dast_watched_name, dast_watched_size * sizeof(char *));
	dast_watched_callback =  realloc(dast_watched_callback, dast_watched_size * sizeof(callback_func));
	dast_watched_pidfile = realloc(dast_watched_pidfile, dast_watched_size * sizeof(FILE *));

	dast_watched_name[dast_watched_size - 1] = malloc(strlen(filename) + 1);
	strcpy(dast_watched_name[dast_watched_size - 1], filename);

	dast_watched_callback[dast_watched_size - 1] = func;

	pidfile_name = generate_pidfile_name(filename);
	open_rw(pidfile_name, &dast_watched_pidfile[dast_watched_size - 1]);
	free(pidfile_name);
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
				
				if(iev->mask & IN_CLOSE_WRITE){   // || iev->mask & IN_MODIFY
					for(int a = 0; a < dast_watched_size; a ++){
						if(strcmp(dast_watched_name[a], iev->name) == 0){
							pid_t pid = dast_read_pid(dast_watched_pidfile[a]);
							printf("PID: %d\n", pid);
							if(pid > 0){
								if(pid != parent_pid) (*dast_watched_callback[a])();
							}
							else{
								perror("read pid");
							}
						}
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
		printf("PID of parent %d\n", cpid);
		parent_pid = cpid;
		//exit(EXIT_SUCCESS);
	}

}

s_byte dast_watch_dir(char * dir_name){
	if(inotify_add_watch(ifd, dir_name, IN_CLOSE_WRITE) < 0){  // | IN_CLOSE_NOWRITE | IN_MODIFY
		return -1;
	}

	return 0;
}


void dast_cleanup(){
	unsigned int pos = 0;

	kill(0, SIGKILL);   /* kill child */
	wait(NULL);			 /* Wait for child */

	for(pos = 0; pos < sizeof(dast_watched_name); pos ++){
		free(dast_watched_name[pos]);
		fclose(dast_watched_pidfile[pos]);
	}
	free(dast_watched_name);

	free(dast_watched_callback);
	free(dast_watched_pidfile);
}